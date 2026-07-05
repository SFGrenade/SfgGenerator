// Header assigned to this source
#include "common/NoteMap.hpp"

// Project includes
#include "libraryExtensions/math.hpp"

// C++ std includes
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <exception>
#include <functional>
#include <numbers>

bool NoteMap::NoteDescription::operator==( NoteMap::NoteDescription const& b ) const {
  return ( this->portIndex == b.portIndex ) && ( this->channelId == b.channelId ) && ( this->key == b.key ) && ( this->noteId == b.noteId );
}

bool NoteMap::NoteDescription::operator<( NoteMap::NoteDescription const& b ) const {
  return ( ( this->portIndex < b.portIndex ) ) || ( ( this->portIndex == b.portIndex ) && ( this->channelId < b.channelId ) ) || ( ( this->portIndex == b.portIndex ) && ( this->channelId == b.channelId ) && ( this->key < b.key ) ) || ( ( this->portIndex == b.portIndex ) && ( this->channelId == b.channelId ) && ( this->key == b.key ) && ( this->noteId < b.noteId ) );
}

NoteMap::NoteMap( double sampleRate ) : sampleRate_( sampleRate ), sampleTime_( 1.0 / sampleRate ) {}

NoteMap::~NoteMap() {
  std::lock_guard< std::mutex > lock( noteMapMutex_ );
  noteMap_.clear();
}

void NoteMap::setSampleRate( double sampleRate ) {
  sampleRate_ = sampleRate;
  sampleTime_ = 1.0 / sampleRate;
}

void NoteMap::setAdsrParameters( double attack, double decay, double sustain, double release ) {
  attack_ = attack;
  decay_ = decay;
  sustain_ = sustain;
  release_ = release;
}

void NoteMap::clear() {
  std::lock_guard< std::mutex > lock( noteMapMutex_ );
  noteMap_.clear();
}

double NoteMap::velocity( NoteMap::NoteDescription const& note ) const {
  std::lock_guard< std::mutex > lock( noteMapMutex_ );
  for( auto iter = noteMap_.begin(); iter != noteMap_.end(); ) {
    if( ( note.portIndex != -1 ) && ( note.portIndex != iter->first.portIndex ) ) {
      iter++;
      continue;
    }
    if( ( note.channelId != -1 ) && ( note.channelId != iter->first.channelId ) ) {
      iter++;
      continue;
    }
    if( ( note.key != -1 ) && ( note.key != iter->first.key ) ) {
      iter++;
      continue;
    }
    if( ( note.noteId != -1 ) && ( note.noteId != iter->first.noteId ) ) {
      iter++;
      continue;
    }
    return iter->second.velocity;
  }
  return 0.0;
}

void NoteMap::foreach( std::function< void( std::pair< NoteMap::NoteDescription const, NoteMap::NoteData >& entry ) > const& callback ) {
  std::lock_guard< std::mutex > lock( noteMapMutex_ );
  for( auto iter = noteMap_.begin(); iter != noteMap_.end(); ) {
    updateEnvelope( iter->second );
    callback( *iter );
    if( iter->second.envelopePhase == NoteMap::NoteData::EnvelopePhase::Off ) {
      // delete note if it's off
      iter = noteMap_.erase( iter );
    } else {
      iter++;
    }
  }
}

void NoteMap::handleEvent( clap_event_note_t const* ev ) {
  if( ev->header.space_id != CLAP_CORE_EVENT_SPACE_ID ) {
    return;
  }
  if( ( ev->header.type != CLAP_EVENT_NOTE_ON ) && ( ev->header.type != CLAP_EVENT_NOTE_OFF ) && ( ev->header.type != CLAP_EVENT_NOTE_CHOKE ) && ( ev->header.type != CLAP_EVENT_NOTE_END ) ) {
    return;
  }
  NoteMap::NoteDescription note;
  note.portIndex = ev->port_index;
  note.channelId = ev->channel;
  note.key = ev->key;
  note.noteId = ev->note_id;
  if( ev->header.type == CLAP_EVENT_NOTE_ON ) {
    handleNoteOn( note, ev->velocity );
  } else if( ev->header.type == CLAP_EVENT_NOTE_OFF ) {
    handleNoteOff( note, false );
  } else if( ev->header.type == CLAP_EVENT_NOTE_CHOKE ) {
    handleNoteOff( note, true );
  } else if( ev->header.type == CLAP_EVENT_NOTE_END ) {
    handleNoteOff( note, false );
  }
}

void NoteMap::handleEvent( clap_event_midi_t const* ev ) {
  if( ev->header.space_id != CLAP_CORE_EVENT_SPACE_ID ) {
    return;
  }
  if( ( ev->header.type != CLAP_EVENT_MIDI ) ) {
    return;
  }
  uint8_t event = ev->data[0] & 0xF0;
  if( ( event != 0x80 ) && ( event != 0x90 ) && ( event != 0xA0 ) ) {
    return;
  }
  NoteMap::NoteDescription note;
  note.portIndex = int16_t( ev->port_index );
  note.channelId = uint8_t( ev->data[0] & 0x0F );
  note.key = uint8_t( ev->data[1] & 0x7F );
  note.noteId = 0;
  double velocity = double( ev->data[2] & 0x7F ) / double( 0x7F );
  if( event == 0x80 ) {
    // Note off
    handleNoteOff( note, false );
  } else if( event == 0x90 ) {
    // Note on
    handleNoteOn( note, velocity );
  } else if( event == 0xA0 ) {
    // Note aftertouch
    handleNoteVelocityChange( note, velocity );
  }
}

void NoteMap::handleNoteOn( NoteMap::NoteDescription const& note, double velocity ) {
  std::lock_guard< std::mutex > lock( noteMapMutex_ );
  if( ( ( note.portIndex != -1 ) && ( note.channelId != -1 ) && ( note.key != -1 ) && ( note.noteId != -1 ) ) && ( noteMap_.find( note ) == noteMap_.end() ) ) {
    NoteMap::NoteData tmp;
    tmp.attack = attack_;
    tmp.decay = decay_;
    tmp.sustain = sustain_;
    tmp.release = release_;
    noteMap_.insert( { note, tmp } );
  }
  for( auto iter = noteMap_.begin(); iter != noteMap_.end(); ) {
    if( ( note.portIndex != -1 ) && ( note.portIndex != iter->first.portIndex ) ) {
      iter++;
      continue;
    }
    if( ( note.channelId != -1 ) && ( note.channelId != iter->first.channelId ) ) {
      iter++;
      continue;
    }
    if( ( note.key != -1 ) && ( note.key != iter->first.key ) ) {
      iter++;
      continue;
    }
    if( ( note.noteId != -1 ) && ( note.noteId != iter->first.noteId ) ) {
      iter++;
      continue;
    }
    iter->second.phase = 0.0;
    iter->second.velocity = velocity;
    iter->second.envelopePhase = NoteMap::NoteData::EnvelopePhase::Attack;
    iter->second.envelopeTime = 0.0;
    iter->second.envelopeLevel = 0.0;
    iter++;
  }
}

void NoteMap::handleNoteVelocityChange( NoteMap::NoteDescription const& note, double velocity ) {
  std::lock_guard< std::mutex > lock( noteMapMutex_ );
  for( auto iter = noteMap_.begin(); iter != noteMap_.end(); ) {
    if( ( note.portIndex != -1 ) && ( note.portIndex != iter->first.portIndex ) ) {
      iter++;
      continue;
    }
    if( ( note.channelId != -1 ) && ( note.channelId != iter->first.channelId ) ) {
      iter++;
      continue;
    }
    if( ( note.key != -1 ) && ( note.key != iter->first.key ) ) {
      iter++;
      continue;
    }
    if( ( note.noteId != -1 ) && ( note.noteId != iter->first.noteId ) ) {
      iter++;
      continue;
    }
    iter->second.velocity = velocity;
    iter++;
  }
}

void NoteMap::handleNoteOff( NoteMap::NoteDescription const& note, bool choke ) {
  std::lock_guard< std::mutex > lock( noteMapMutex_ );
  for( auto iter = noteMap_.begin(); iter != noteMap_.end(); ) {
    if( ( note.portIndex != -1 ) && ( note.portIndex != iter->first.portIndex ) ) {
      iter++;
      continue;
    }
    if( ( note.channelId != -1 ) && ( note.channelId != iter->first.channelId ) ) {
      iter++;
      continue;
    }
    if( ( note.key != -1 ) && ( note.key != iter->first.key ) ) {
      iter++;
      continue;
    }
    if( ( note.noteId != -1 ) && ( note.noteId != iter->first.noteId ) ) {
      iter++;
      continue;
    }
    // Start release phase instead of removing immediately
    if( choke ) {
      if( iter->second.envelopePhase != NoteMap::NoteData::EnvelopePhase::Off ) {
        iter->second.envelopePhase = NoteMap::NoteData::EnvelopePhase::Off;
        iter->second.envelopeTime = 0.0;
      }
    } else {
      if( iter->second.envelopePhase != NoteMap::NoteData::EnvelopePhase::Release ) {
        iter->second.envelopePhase = NoteMap::NoteData::EnvelopePhase::Release;
        iter->second.envelopeTime = 0.0;
      }
    }
    iter++;
  }
}

void NoteMap::updateEnvelope( NoteData& noteData ) {
  // If note is off, keep envelope at 0
  if( noteData.envelopePhase == NoteMap::NoteData::EnvelopePhase::Off ) {
    noteData.envelopeLevel = 0.0;
    noteData.envelopeTime = 0.0;
    return;
  }

  // Update time in current phase
  noteData.envelopeTime += sampleTime_;

  // Handle each phase
  if( noteData.envelopePhase == NoteMap::NoteData::EnvelopePhase::Attack ) {
    if( noteData.envelopeTime >= noteData.attack ) {
      noteData.envelopeLevel = 1.0;
      noteData.envelopePhase = ( noteData.decay > 0.0 ) ? NoteMap::NoteData::EnvelopePhase::Decay : NoteMap::NoteData::EnvelopePhase::Sustain;  // Go to decay or sustain
      noteData.envelopeTime = 0.0;
    } else {
      noteData.envelopeLevel = noteData.envelopeTime / noteData.attack;
    }
  } else if( noteData.envelopePhase == NoteMap::NoteData::EnvelopePhase::Decay ) {
    if( noteData.envelopeTime >= noteData.decay ) {
      noteData.envelopeLevel = noteData.sustain;
      noteData.envelopePhase = NoteMap::NoteData::EnvelopePhase::Sustain;
      noteData.envelopeTime = 0.0;
    } else {
      noteData.envelopeLevel = std::lerp( 1.0, noteData.sustain, noteData.envelopeTime / noteData.decay );
    }
  } else if( noteData.envelopePhase == NoteMap::NoteData::EnvelopePhase::Sustain ) {
    noteData.envelopeLevel = noteData.sustain;
  } else if( noteData.envelopePhase == NoteMap::NoteData::EnvelopePhase::Release ) {
    if( noteData.envelopeTime >= noteData.release ) {
      noteData.envelopeLevel = 0.0;
      noteData.envelopePhase = NoteMap::NoteData::EnvelopePhase::Off;  // Go to off
      noteData.envelopeTime = 0.0;
    } else {
      noteData.envelopeLevel = std::lerp( noteData.envelopeLevel, 0.0, noteData.envelopeTime / noteData.release );
    }
  }
}
