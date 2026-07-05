#pragma once

// Project includes
#include "libraryExtensions/_clap.hpp"
#include "libraryExtensions/logging.hpp"

// C++ std includes
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>

class NoteMap {
  public:
  // see `clap_event_note_t`, this is a map of key to note
  struct NoteDescription {
    int16_t portIndex = -1;  // port index from ext/note-ports; -1 for wildcard
    int16_t channelId = -1;  // 0..15, same as MIDI1 Channel Number, -1 for wildcard
    int16_t key = -1;        // 0..127, same as MIDI1 Key Number (60==Middle C), -1 for wildcard
    int32_t noteId = -1;     // host provided note id >= 0, or -1 if unspecified or wildcard
    bool operator==( NoteMap::NoteDescription const& b ) const;
    bool operator<( NoteMap::NoteDescription const& b ) const;
    bool operator!=( NoteMap::NoteDescription const& b ) const { return !( *this == b ); };
    bool operator>( NoteMap::NoteDescription const& b ) const { return b < *this; }
    bool operator<=( NoteMap::NoteDescription const& b ) const { return !( *this > b ); }
    bool operator>=( NoteMap::NoteDescription const& b ) const { return !( *this < b ); }
  };
  struct NoteData {
    enum class EnvelopePhase { Off, Attack, Decay, Sustain, Release };
    double phase = 0.0;
    double velocity = 0.0;  // aka amplitude

    // ADSR envelope parameters (in seconds)
    double attack = 0.0;
    double decay = 1.0;
    double sustain = 1.0;
    double release = 0.0;

    // ADSR envelope state
    double envelopeLevel = 0.0;
    EnvelopePhase envelopePhase = EnvelopePhase::Off;
    double envelopeTime = 0.0;  // time spent in current phase
  };

  public:
  NoteMap( double sampleRate = 44100.0 );
  virtual ~NoteMap();

  void setSampleRate( double sampleRate );
  void setAdsrParameters( double attack, double decay, double sustain, double release );
  void clear();
  double velocity( NoteMap::NoteDescription const& note ) const;
  void foreach( std::function< void( std::pair< NoteMap::NoteDescription const, NoteMap::NoteData >& entry ) > const& callback );
  void handleEvent( clap_event_note_t const* ev );
  void handleEvent( clap_event_midi_t const* ev );

  private:
  void handleNoteOn( NoteMap::NoteDescription const& note, double velocity );
  void handleNoteVelocityChange( NoteMap::NoteDescription const& note, double velocity );
  void handleNoteOff( NoteMap::NoteDescription const& note, bool choke );

  private:
  void updateEnvelope( NoteData& noteData );

  private:
  std::map< NoteDescription, NoteData > noteMap_;
  mutable std::mutex noteMapMutex_;
  double sampleRate_ = 44100.0;
  double sampleTime_ = 1.0 / 44100.0;

  // ADSR envelope parameters (in seconds) for the notes
  double attack_ = 0.0;
  double decay_ = 1.0;
  double sustain_ = 1.0;
  double release_ = 0.0;
};
