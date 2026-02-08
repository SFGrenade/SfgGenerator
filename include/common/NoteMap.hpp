#pragma once

// Project includes
#include "common/_clap.hpp"
#include "common/_fmt.hpp"

// C++ std includes
#include <cstdint>
#include <functional>
#include <map>

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
    double phase = 0.0;
    double velocity = 0.0;  // aka amplitude
  };

  public:
  NoteMap();
  virtual ~NoteMap();

  void clear();
  void foreach( std::function< void( std::pair< NoteMap::NoteDescription const, NoteMap::NoteData >& entry ) > const& callback );
  void handleEvent( clap_event_note_t const* ev );
  void handleEvent( clap_event_midi_t const* ev );

  private:
  void handleNoteOn( NoteMap::NoteDescription const& note, double velocity );
  void handleNoteVelocityChange( NoteMap::NoteDescription const& note, double velocity );
  void handleNoteOff( NoteMap::NoteDescription const& note );

  private:
  std::map< NoteDescription, NoteData > noteMap_;
};
