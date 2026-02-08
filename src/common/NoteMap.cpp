// Header assigned to this source
#include "common/NoteMap.hpp"

// Project includes
#include "common/math.hpp"

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
  return ( ( this->portIndex < b.portIndex ) ) || ( ( this->portIndex == b.portIndex ) && ( this->channelId < b.channelId ) )
         || ( ( this->portIndex == b.portIndex ) && ( this->channelId == b.channelId ) && ( this->key < b.key ) )
         || ( ( this->portIndex == b.portIndex ) && ( this->channelId == b.channelId ) && ( this->key == b.key ) && ( this->noteId < b.noteId ) );
}

NoteMap::NoteMap() {}

NoteMap::~NoteMap() {
  noteMap_.clear();
}

void NoteMap::clear() {
  noteMap_.clear();
}

void NoteMap::foreach( std::function< void( std::pair< NoteMap::NoteDescription const, NoteMap::NoteData >& entry ) > const& callback ) {
  for( std::pair< NoteMap::NoteDescription const, NoteMap::NoteData >& entry : noteMap_ ) {
    callback( entry );
  }
}

void NoteMap::handleEvent( clap_event_note_t const* ev ) {
  if( ev->header.space_id != CLAP_CORE_EVENT_SPACE_ID ) {
    return;
  }
  if( ( ev->header.type != CLAP_EVENT_NOTE_ON ) && ( ev->header.type != CLAP_EVENT_NOTE_OFF ) && ( ev->header.type != CLAP_EVENT_NOTE_CHOKE )
      && ( ev->header.type != CLAP_EVENT_NOTE_END ) ) {
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
    handleNoteOff( note );
  } else if( ev->header.type == CLAP_EVENT_NOTE_CHOKE ) {
    handleNoteOff( note );
  } else if( ev->header.type == CLAP_EVENT_NOTE_END ) {
    handleNoteOff( note );
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
    handleNoteOff( note );
  } else if( event == 0x90 ) {
    // Note on
    handleNoteOn( note, velocity );
  } else if( event == 0xA0 ) {
    // Note aftertouch
    handleNoteVelocityChange( note, velocity );
  }
}

void NoteMap::handleNoteOn( NoteMap::NoteDescription const& note, double velocity ) {
  if( ( ( note.portIndex != -1 ) && ( note.channelId != -1 ) && ( note.key != -1 ) && ( note.noteId != -1 ) ) && ( noteMap_.find( note ) == noteMap_.end() ) ) {
    noteMap_.insert( { note, NoteMap::NoteData() } );
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
    iter++;
  }
}

void NoteMap::handleNoteVelocityChange( NoteMap::NoteDescription const& note, double velocity ) {
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

void NoteMap::handleNoteOff( NoteMap::NoteDescription const& note ) {
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
    iter = noteMap_.erase( iter );
  }
}
