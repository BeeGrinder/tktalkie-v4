/****
 * Routines to handle reading/handling settings
 */

void ConfigureButton(byte a) {
  
    Settings.glove.ControlButtons[a].setPTT(false);
    Settings.glove.ControlButtons[a].setPin(0);
    byte buttonNum = 0;
    char buf[25];
    byte pin = Config.buttons[a];
    
    Serial.print("** Configure Button for Pin: ");
    Serial.println(pin);
    if (pin > 0) {

      // setup physical button
      Settings.glove.ControlButtons[a].setup(pin);
      
      strcpy(buf, Settings.glove.settings[a]);
      char *part_token, *part_ptr;
      Serial.print("-- SETTINGS: ");
      Serial.println(buf);
      part_token = strtok_r(buf, ";", &part_ptr);
      // button_type,data(sound)
      Serial.print("Initial part token: ");
      Serial.println(part_token);
      byte b = 0;

      while (part_token && b < 2) {
        char *button_token, *button_ptr;
        button_token = strtok_r(part_token, ",", &button_ptr);
        Serial.print("Intitial button token: ");
        Serial.println(button_token);
        byte button_type = (byte)atoi(button_token);
        Serial.print("Button Type: ");
        Serial.println(button_type);
        // by default, do not continue processing
        byte max = 0;
        // Determine how many options we need to read 
        // based on the type of button
        switch (button_type) {
          // PTT/Sleep/Wake Button
          case 1:
            {
              debug(F("PTT Button on pin: %d\n"), pin);
              Serial.println(" -> PTT Button");
              App.ptt_button = a;
              Settings.glove.ControlButtons[a].setPTT(true);
              if (App.wake_button == NULL) {
                App.wake_button = a;
                snoozeDigital.pinMode(pin, INPUT_PULLUP, FALLING);
              }
            }  
            break;
          // Sound button    
          case 2:
            {
              max = 2;
            }  
            break;
          // sleep/wake (overrides PTT)
          case 6:
            {
              debug(F("Sleep Button on pin: %d\n"), pin);
              Serial.print("Sleep Button on pin: ");
              Serial.println(pin);
              App.wake_button = a;
              snoozeDigital.pinMode(pin, INPUT_PULLUP, FALLING);
            }  
            break;   
        }

        // setup virtual button type
        Serial.print("Setting phyical button ");
        Serial.print(a);
        Serial.print(", Virtual button ");
        Serial.print(buttonNum);
        Serial.print(" to ");
        Serial.print(button_type);
        Serial.print(" max: ");
        Serial.println(max);
        
        Settings.glove.ControlButtons[a].buttons[buttonNum].setup(button_type);
        
        // start off with one since we have the first part 
        // and just need to get the second part before 
        // we keep processing the settings
        byte c = 1;
        button_token = strtok_r(NULL, ",", &button_ptr);
        
        while (button_token && c < max) {
          switch (button_type) {
            case 2:
              {
                debug(F("Sound Button on pin: %d\n"), pin);
                Serial.println(" -> Sound Button");
                Serial.print("Setting sound to: ");
                Serial.println(button_token);
                Settings.glove.ControlButtons[a].buttons[buttonNum].setSound(button_token);
              }  
              break;
          }

          // NOTE: Each physical button needs to have 
          // to virtual button properties that hold 
          // what to do
          
          Serial.print("token -> ");
          Serial.print(a);
          Serial.print(" -> ");
          Serial.print(b);
          Serial.print(" -> ");
          Serial.print(c);
          Serial.print(" -> ");
          Serial.println(button_token);
          //byte cb_type = atoi(token);
          c++;
          button_token = strtok_r(NULL, ",", &button_ptr);
        }  
        buttonNum++;
        b++;
        part_token = strtok_r(NULL, ";", &part_ptr);
      }
    }
    Serial.println("--------------END OF BUTTON-----------------");
    Serial.println("");
}

void setVolume() {
  if (Settings.volume.master > 1) { 
    Settings.volume.master = 1;
  } else if (Settings.volume.master < 0) {
    Settings.volume.master = 0;
  }
  //audioShield.volume(readVolume());
  char buf[30];
  dtostrf(Settings.volume.master, 0, 3, buf);
  debug(F("VOLUME: %s\n"), buf);
  audioShield.volume(Settings.volume.master);
}

void setMicGain() {
  audioShield.micGain(Settings.volume.microphone); 
}

void setLineout() {
  if (Settings.volume.lineout < 13) {
    Settings.volume.lineout = 13;  
  } else if (Settings.volume.lineout > 31) {
    Settings.volume.lineout = 31;
  }
  debug(F("LINEOUT: %d\n"), Settings.volume.lineout);
  audioShield.lineOutLevel(Settings.volume.lineout);
}

void setLinein() {
  if (Settings.volume.linein < 0) {
    Settings.volume.linein = 0;  
  } else if (Settings.volume.linein > 15) {
    Settings.volume.linein = 15;
  }  
  debug(F("LINEIN: %d\n"), Settings.volume.linein);
  audioShield.lineInLevel(Settings.volume.linein);
}

void setHipass() {
  if (Settings.effects.highpass < 0) { 
    Settings.effects.highpass = 0;
  } else if (Settings.effects.highpass > 1) {
    Settings.effects.highpass = 1;
  }
  debug(F("HIGHPASS: %d\n"), Settings.effects.highpass);
  if (Settings.effects.highpass == 0) {
    audioShield.adcHighPassFilterDisable();
  } else {
    audioShield.adcHighPassFilterEnable();
  }
}

void setNoiseVolume() {
  char buf[30];
  dtostrf(Settings.effects.noise, 0, 3, buf);
  debug(F("NOISE: %s\n"),  buf);
  effectsMixer.gain(3, Settings.effects.noise);
}

void setVoiceVolume() {
  char buf[30];
  dtostrf(Settings.voice.volume, 0, 3, buf);
  debug(F("VOICE: %s\n"),  buf);
  voiceMixer.gain(0, Settings.voice.volume);
  //voiceMixer.gain(1, Settings.voice.volume);
}

void setDryVolume() {
  char buf[30];
  dtostrf(Settings.voice.dry, 0, 3, buf);
  debug(F("DRY: %s\n"),  buf);
  voiceMixer.gain(2, Settings.voice.dry);
}

void setEffectsVolume() {
  char buf[30];
  dtostrf(Settings.effects.volume, 0, 3, buf);
  debug(F("EFFECTS VOLUME: %s\n"),  buf);
  effectsMixer.gain(0, Settings.effects.volume);
  //effectsMixer.gain(1, Settings.effects.volume);
  // Waveform (BLE) connect sound
  effectsMixer.gain(2, Settings.effects.volume);
}

void setLoopVolume() {
  if (Settings.loop.volume < 0 or Settings.loop.volume > 32767) {
    Settings.loop.volume = 4;
  }
  // chatter loop from SD card
  effectsMixer.gain(1, Settings.loop.volume);
  //loopMixer.gain(0, Settings.loop.volume);
  //loopMixer.gain(1, Settings.loop.volume);
}

void setEq() {
  if (Settings.eq.active < 0) {
    Settings.eq.active = 0;
  } else if (Settings.eq.active > 1) {
    Settings.eq.active = 1;
  }
  // Turn on the 5-band graphic equalizer (there is also a 7-band parametric...see the Teensy docs)
  if (Settings.eq.active == 0) {
    audioShield.eqSelect(FLAT_FREQUENCY);
  } else {
    audioShield.eqSelect(GRAPHIC_EQUALIZER);
  }  
}

void setEqBands() {
  // Bands (from left to right) are: Low, Low-Mid, Mid, High-Mid, High.
  // Valid values are -1 (-11.75dB) to 1 (+12dB)
  // The settings below pull down the lows and highs and push up the mids for 
  // more of a "tin-can" sound.
  audioShield.eqBands(Settings.eq.bands[0], Settings.eq.bands[1], Settings.eq.bands[2], Settings.eq.bands[3], Settings.eq.bands[4]);
}

void setBitcrusher() {
  // You can modify these values to process the voice 
  // input.  See the Teensy bitcrusher demo for details.
  bitcrusher1.bits(Settings.effects.bitcrusher.bits);
  bitcrusher1.sampleRate(Settings.effects.bitcrusher.rate);
}

void setEffectsDir() {
  fixPath(Settings.effects.dir);
  loadSoundEffects();
}

void setSoundsDir() {
  fixPath(Settings.sounds.dir);
}

void setLoopDir() {
  fixPath(Settings.loop.dir);
}

void setLoopMute() {
  if (Settings.loop.mute > 1) {
    Settings.loop.mute = 1;
  } else if (Settings.loop.mute < 0) {
    Settings.loop.mute = 0;
  }
}

void setEffectsMute() {
  if (Settings.effects.mute > 1) {
    Settings.effects.mute = 1;
  } else if (Settings.effects.mute < 0) {
    Settings.effects.mute = 0;
  }
}

void setSleepTimer() {
  if (Settings.sleep.timer < 0) {
    Settings.sleep.timer = 0;
  }
}

void setChorus() {
  if (Settings.effects.chorus.delay > 32) {
      Settings.effects.chorus.delay = 32;
    }
    if (Settings.effects.chorus.delay < 1) {
      Settings.effects.chorus.delay = 1;
    }
    if (Settings.effects.chorus.voices < 0) {
      Settings.effects.chorus.voices = 0;
    }
    debug(F("CHORUS: %d - %d"), Settings.effects.chorus.voices, Settings.effects.chorus.delay);
    if (Settings.effects.chorus.voices < 1) {
      chorus1.voices(0);
    } else if(!chorus1.begin(Settings.effects.chorus.buffer,Settings.effects.chorus.delay*AUDIO_BLOCK_SAMPLES,Settings.effects.chorus.voices)) {
       Serial.println("chorus: startup failed");
    }
}

void setFlanger() {
  if (Settings.effects.flanger.delay > 32) {
    Settings.effects.flanger.delay = 32;
  }
  if (Settings.effects.flanger.delay < 0) {
    Settings.effects.flanger.delay = 0;
  }
  flange1.begin(Settings.effects.flanger.buffer,Settings.effects.flanger.delay*AUDIO_BLOCK_SAMPLES,Settings.effects.flanger.offset,Settings.effects.flanger.depth,Settings.effects.flanger.freq); 
}
/***
 * Parse and set a Configuration setting
 */
void parseSetting(const char *settingName, char *settingValue) 
{

  debug(F("Parse Setting: %s = %s\n"), settingName, settingValue);

  if (strcasecmp(settingName, "name") == 0) {
    memset(Settings.name, 0, sizeof(Settings.name));
    strcpy(Settings.name, settingValue);
  } else if (strcasecmp(settingName, "volume") == 0) {
    Settings.volume.master = atof(settingValue);
    setVolume();  
  } else if (strcasecmp(settingName, "lineout") == 0) {
    Settings.volume.lineout = (byte)atoi(settingValue);
    setLineout();
  } else if (strcasecmp(settingName, "linein") == 0) {
    Settings.volume.linein = (byte)atoi(settingValue);
    setLinein();
  } else if ((strcasecmp(settingName, "high_pass") == 0) || (strcasecmp(settingName, "highpass") == 0)) {
    Settings.effects.highpass = (byte)atoi(settingValue);
    setHipass();
  } else if (strcasecmp(settingName, "microphone") == 0 || strcasecmp(settingName, "mic_gain") == 0) {
    Settings.volume.microphone = atoi(settingValue);
    setMicGain();
  } else if (strcasecmp(settingName, "button_click") == 0) {
    memset(Settings.sounds.button, 0, sizeof(Settings.sounds.button));
    strcpy(Settings.sounds.button, settingValue);
  } else if ( (strcasecmp(settingName, "startup") == 0) || (strcasecmp(settingName, "startup_sound") == 0) ) {
    memset(Settings.sounds.start, 0, sizeof(Settings.sounds.start));
    strcpy(Settings.sounds.start, settingValue);
  } else if ( (strcasecmp(settingName, "loop") == 0) || (strcasecmp(settingName, "loop.file") == 0) ) {
    memset(Settings.loop.file, 0, sizeof(Settings.loop.file));
    strcpy(Settings.loop.file, settingValue);
  } else if ( (strcasecmp(settingName, "noise_gain") == 0) || (strcasecmp(settingName, "noise") == 0) ) {
    Settings.effects.noise = atof(settingValue);
    setNoiseVolume();
  } else if ( (strcasecmp(settingName, "voice_gain") == 0) || (strcasecmp(settingName, "voice.volume") == 0) ) {
    Settings.voice.volume = atof(settingValue);
    setVoiceVolume();
  } else if (strcasecmp(settingName, "dry_gain") == 0) {
    Settings.voice.dry = atof(settingValue);  
    setDryVolume();
  } else if (strcasecmp(settingName, "effects_gain") == 0) {
    Settings.effects.volume = atof(settingValue);
    setEffectsVolume();
  } else if (strcasecmp(settingName, "loop_gain") == 0) {
    Settings.loop.volume = atof(settingValue);
    setLoopVolume();
  } else if (strcasecmp(settingName, "silence_time") == 0) {
    Settings.voice.wait = atoi(settingValue);
  } else if (strcasecmp(settingName, "voice_start") == 0) {
    Settings.voice.start = atof(settingValue);
  } else if (strcasecmp(settingName, "voice_stop") == 0) {  
    Settings.voice.stop = atof(settingValue);
  } else if (strcasecmp(settingName, "eq") == 0) {
    Settings.eq.active = (byte)atoi(settingValue);
    setEq();
  } else if (strcasecmp(settingName, "eq_bands") == 0) {
    // clear bands and prep for setting
    for (int i = 0; i < 6; i++) {
      Settings.eq.bands[i] = 0;
    }
    char *band, *ptr;
    band = strtok_r(settingValue, ",", &ptr);
    byte i = 0;
    while (band && i < 6) {
      Settings.eq.bands[i] = atof(band);
      i++;
      band = strtok_r(NULL, ",", &ptr);
    }
    setEqBands();
  } else if (strcasecmp(settingName, "bitcrushers") == 0 || strcasecmp(settingName, "bitcrusher") == 0) {
    char *token, *ptr;
    token = strtok_r(settingValue, ",", &ptr);
    byte i = 0;
    while (token && i < (3)) {
      switch (i) {
        case 0:
           Settings.effects.bitcrusher.bits = atoi(token);
           break;
        case 1:
           Settings.effects.bitcrusher.rate = atoi(token);
           break;
      }
      i++;
      token = strtok_r(NULL, ",", &ptr);
    }
    setBitcrusher();
  } else if (strcasecmp(settingName, "effects_dir") == 0) {
    memset(Settings.effects.dir, 0, sizeof(Settings.effects.dir));
    strcpy(Settings.effects.dir, settingValue);
    setEffectsDir();
  } else if (strcasecmp(settingName, "sounds_dir") == 0) {
    memset(Settings.sounds.dir, 0, sizeof(Settings.sounds.dir));
    strcpy(Settings.sounds.dir, settingValue);
    setSoundsDir();
  } else if (strcasecmp(settingName, "loop_dir") == 0) {
    memset(Settings.loop.dir, 0, sizeof(Settings.loop.dir));
    strcpy(Settings.loop.dir, settingValue);
    setLoopDir();
  } else if (strcasecmp(settingName, "mute_loop") == 0) {
    Settings.loop.mute = (byte)atoi(settingValue);
    setLoopMute();
  } else if (strcasecmp(settingName, "mute_effects") == 0) {
    Settings.effects.mute = (byte)atoi(settingValue);
    setEffectsMute();
  } else if (strcasecmp(settingName, "sleep_time") == 0) {
    Settings.sleep.timer = (byte)atoi(settingValue);
    setSleepTimer();
  } else if (strcasecmp(settingName, "sleep_sound") == 0) {
    memset(Settings.sleep.file, 0, sizeof(Settings.sleep.file));
    strcpy(Settings.sleep.file, settingValue);
  } else if (strcasecmp(settingName, "chorus") == 0) {
    if (strcasecmp(settingValue, "0") == 0) {
      chorus1.voices(0);
    } else if (strcasecmp(settingValue, "1") == 0) {
      chorus1.voices(Settings.effects.chorus.voices);
    } else {
      char *token, *ptr;
      token = strtok_r(settingValue, ",", &ptr);
      byte i = 0;
      while (token && i < 3) {
        switch (i) {
          case 0:
            Settings.effects.chorus.voices = (byte)atoi(token);
            break;
          case 1:
            Settings.effects.chorus.delay = (byte)atoi(token);
            break;
        }
        i++;
        token = strtok_r(NULL, ",", &ptr);
      }  
    }
    setChorus();
  } else if (strcasecmp(settingName, "chorus_delay") == 0) {
      Settings.effects.chorus.delay = (byte)atoi(settingValue);
      setChorus();
  } else if (strcasecmp(settingName, "chorus_voices") == 0) {
      Settings.effects.chorus.voices = (byte)atoi(settingValue);
      if (Settings.effects.chorus.voices < 0) {
        Settings.effects.chorus.voices = 0;
      }
      chorus1.voices(Settings.effects.chorus.voices);
  } else if (strcasecmp(settingName, "flanger_delay") == 0) {
      Settings.effects.flanger.delay = (byte)atoi(settingValue);
      setFlanger();   
  } else if (strcasecmp(settingName, "flanger_freq") == 0) {
      Settings.effects.flanger.freq = atof(settingValue);
      if (Settings.effects.flanger.freq < 0) {
        Settings.effects.flanger.freq = 0;
      }
      if (Settings.effects.flanger.freq > 10) {
        Settings.effects.flanger.freq = 10;
      }
      flange1.voices(Settings.effects.flanger.offset,Settings.effects.flanger.depth,Settings.effects.flanger.freq);
  } else if (strcasecmp(settingName, "flanger_depth") == 0) {
      Settings.effects.flanger.depth = (byte)atoi(settingValue);
      if (Settings.effects.flanger.depth < 0) {
        Settings.effects.flanger.depth = 0;
      }
      if (Settings.effects.flanger.depth > 255) {
        Settings.effects.flanger.depth = 255;
      }
      flange1.voices(Settings.effects.flanger.offset,Settings.effects.flanger.depth,Settings.effects.flanger.freq);
  } else if (strcasecmp(settingName, "flanger_offset") == 0) {
      Settings.effects.flanger.offset = (byte)atoi(settingValue);// * Settings.effects.flanger.delay_LENGTH;
      if (Settings.effects.flanger.offset < 1) {
        Settings.effects.flanger.offset = 1;
      }
      if (Settings.effects.flanger.offset > 128) {
        Settings.effects.flanger.offset = 128;
      }
      flange1.voices(Settings.effects.flanger.offset,Settings.effects.flanger.depth,Settings.effects.flanger.freq);
  } else if (strcasecmp(settingName, "flanger") == 0) {
      if (strcasecmp(settingValue, "0") == 0) {
        flange1.voices(FLANGE_DELAY_PASSTHRU,0,0);
      } else if (strcasecmp(settingValue, "1") == 0) {
        flange1.voices(Settings.effects.flanger.offset,Settings.effects.flanger.depth,Settings.effects.flanger.freq);
      } else {
        char *token, *ptr;
        token = strtok_r(settingValue, ",", &ptr);
        byte i = 0;
        while (token && i < 3) {
          switch (i) {
            case 0:
              Settings.effects.flanger.offset = (byte)atoi(token);
              break;
            case 1:
              Settings.effects.flanger.depth = (byte)atoi(token);
              break;
            case 2:
              Settings.effects.flanger.freq = atof(token);
              break;
          }
          i++;
          token = strtok_r(NULL, ",", &ptr);
        } 
        flange1.voices(Settings.effects.flanger.offset,Settings.effects.flanger.depth,Settings.effects.flanger.freq); 
      }
  } else if (strcasecmp(settingName, "button") == 0) {
      char *token, *ptr;
      token = strtok_r(settingValue, ",", &ptr);
      byte b = (byte)atoi(token);
      if (b >= 0 && b <= 5) {
        strcpy(Settings.glove.settings[b], ptr);
        ConfigureButton(b);
      }
  } else if (strcasecmp(settingName, "buttons") == 0) {
      char *token, *ptr;
      token = strtok_r(settingValue, "|", &ptr);
      byte a = 0;
      while (token && a < 6) {
        strcpy(Settings.glove.settings[a], token);
        ConfigureButton(a);
        token = strtok_r(NULL, "|", &ptr);
        a++;
      }
  }
  
}

/**
 * Save startup settings
 */
boolean saveConfig() {

  const char filename[13] = "CONFIG.TXT";
  
  debug(F("Saving Config data to %s\n"), filename);
  
  // Open file for writing
  File file = openFile(filename, FILE_WRITE);
  if (!file) {
    Serial.println(F("Failed to create file"));
    return false;
  }

  const size_t bufferSize = JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(6) + 120;
  DynamicJsonBuffer jsonBuffer(bufferSize);

  // Parse the root object
  JsonObject &root = jsonBuffer.createObject();

  // Set the values
  root["profile"] = Config.profile;
  root["access_code"] = Config.access_code;
  root["debug"] = Config.debug;
  root["echo"] = Config.echo;
  root["input"] = Config.input;

  JsonArray& buttons = root.createNestedArray("buttons");
  buttons.add(Config.buttons[0]);
  buttons.add(Config.buttons[1]);
  buttons.add(Config.buttons[2]);
  buttons.add(Config.buttons[3]);
  buttons.add(Config.buttons[4]);
  buttons.add(Config.buttons[5]);

  // Serialize JSON to file
  if (root.prettyPrintTo(file) == 0) {
    Serial.println(F("Failed to write to file"));
    file.close();
    return false;
  } else {
    Serial.println("Config file saved.");
  }

  // Close the file (File's destructor doesn't close the file)
  file.close();
  return true;
  
}

/***
 * Converts all in-memory settings to string
 */
char *settingsToString(char result[], const boolean pretty = false) 
{

  //const size_t bufferSize = JSON_ARRAY_SIZE(5) + JSON_ARRAY_SIZE(6) + 22*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 3*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + JSON_OBJECT_SIZE(9);
  const size_t bufferSize = 6*JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(5) + JSON_ARRAY_SIZE(6) + 4*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 3*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + JSON_OBJECT_SIZE(9) + 780;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.createObject();
  root["name"] = Settings.name;
  
  JsonObject& volume = root.createNestedObject("volume");
  volume["master"] = Settings.volume.master;
  volume["microphone"] = Settings.volume.microphone;
  volume["linein"] = Settings.volume.linein;
  volume["lineout"] = Settings.volume.lineout;
  
  JsonObject& sounds = root.createNestedObject("sounds");
  sounds["dir"] = Settings.sounds.dir;
  sounds["start"] = Settings.sounds.start;
  sounds["button"] = Settings.sounds.button;
  
  JsonObject& loop = root.createNestedObject("loop");
  loop["dir"] = Settings.loop.dir;
  loop["file"] = Settings.loop.file;
  loop["volume"] = Settings.loop.volume;
  loop["mute"] = Settings.loop.mute;
  
  JsonObject& voice = root.createNestedObject("voice");
  voice["volume"] = Settings.voice.volume;
  voice["dry"] = Settings.voice.dry;
  voice["start"] = Settings.voice.start;
  voice["stop"] = Settings.voice.stop;
  voice["wait"] = Settings.voice.wait;
  
  JsonObject& effects = root.createNestedObject("effects");
  effects["dir"] = Settings.effects.dir;
  effects["volume"] = Settings.effects.volume;
  effects["highpass"] = Settings.effects.highpass;
  
  JsonObject& effects_bitcrusher = effects.createNestedObject("bitcrusher");
  effects_bitcrusher["bits"] = Settings.effects.bitcrusher.bits;
  effects_bitcrusher["rate"] = Settings.effects.bitcrusher.rate;
  
  JsonObject& effects_chorus = effects.createNestedObject("chorus");
  effects_chorus["voices"] = Settings.effects.chorus.voices;
  effects_chorus["delay"] = Settings.effects.chorus.delay;
  
  JsonObject& effects_flanger = effects.createNestedObject("flanger");
  effects_flanger["delay"] = Settings.effects.flanger.delay;
  effects_flanger["offset"] = Settings.effects.flanger.offset;
  effects_flanger["depth"] = Settings.effects.flanger.depth;
  effects_flanger["freq"] = Settings.effects.flanger.freq;

  effects["noise"] = Settings.effects.noise;
  effects["mute"] = Settings.effects.mute;
  
  JsonObject& eq = root.createNestedObject("eq");
  eq["active"] = Settings.eq.active;
  
  JsonArray& eq_bands = eq.createNestedArray("bands");
  eq_bands.add(Settings.eq.bands[0]);
  eq_bands.add(Settings.eq.bands[1]);
  eq_bands.add(Settings.eq.bands[2]);
  eq_bands.add(Settings.eq.bands[3]);
  eq_bands.add(Settings.eq.bands[4]);
  
  JsonObject& sleep = root.createNestedObject("sleep");
  sleep["timer"] = Settings.sleep.timer;
  sleep["file"] = Settings.sleep.file;

  JsonObject& glove = root.createNestedObject("glove");
  glove["dir"] = Settings.glove.dir;

  JsonArray& buttons = glove.createNestedArray("buttons");

  char *button;
  
  JsonArray& buttons_0 = buttons.createNestedArray();
  button = Settings.glove.ControlButtons[0].buttons[0].getSettings();
  buttons_0.add(button);
  button = Settings.glove.ControlButtons[0].buttons[1].getSettings();
  buttons_0.add(button);

  JsonArray& buttons_1 = buttons.createNestedArray();
  button = Settings.glove.ControlButtons[1].buttons[0].getSettings();
  buttons_1.add(button);
  button = Settings.glove.ControlButtons[1].buttons[1].getSettings();
  buttons_1.add(button);
  
  JsonArray& buttons_2 = buttons.createNestedArray();
  button = Settings.glove.ControlButtons[2].buttons[0].getSettings();
  buttons_2.add(button);
  button = Settings.glove.ControlButtons[2].buttons[1].getSettings();
  buttons_2.add(button);
  
  JsonArray& buttons_3 = buttons.createNestedArray();
  button = Settings.glove.ControlButtons[3].buttons[0].getSettings();
  buttons_3.add(button);
  button = Settings.glove.ControlButtons[3].buttons[1].getSettings();
  buttons_3.add(button);
  
  JsonArray& buttons_4 = buttons.createNestedArray();
  button = Settings.glove.ControlButtons[4].buttons[0].getSettings();
  buttons_4.add(button);
  button = Settings.glove.ControlButtons[4].buttons[1].getSettings();
  buttons_4.add(button);
  
  JsonArray& buttons_5 = buttons.createNestedArray();
  button = Settings.glove.ControlButtons[5].buttons[0].getSettings();
  buttons_5.add(button);
  button = Settings.glove.ControlButtons[5].buttons[1].getSettings();
  buttons_5.add(button);

  //root.prettyPrintTo(Serial);
  
  if (pretty == true) {
    root.prettyPrintTo((char*)result, root.measurePrettyLength() + 1);
  } else {
    root.printTo((char*)result, root.measureLength() + 1);
  }

  //size_t len = root.measureLength();
  //debug(F("Length: %d\n"), len);
  
  free(button);
  
  return result;

}

/**
 * Backup settings to specified file
 */
boolean saveSettings(const char *src, const boolean backup = true) 
{

  char filename[FILENAME_SIZE];
  boolean result = false;
  if (strcasecmp(src, "") == 0) {
    strcpy(filename, Settings.file);
  } else {
    strcpy(filename, src);
  }
  if (backup == true) {
    addFileExt(filename);
  }
  // add profiles path to file name
  char srcFileName[30];
  strcpy(srcFileName, PROFILES_DIR);
  strcat(srcFileName, filename);
  debug(F("Settings file path: %s\n"), srcFileName);
  if (backup == true) {
    char backupfile[30];
    strcpy(backupfile, PROFILES_DIR);
    strcat(backupfile, filename);
    addBackupExt(backupfile);
    debug(F("Backup File: %s\n"), backupfile);
    File bakFile = openFile(backupfile, FILE_WRITE);
    File srcFile = openFile(srcFileName, FILE_READ);
    if (bakFile && srcFile) {
      char c;
      while (srcFile.available()) {
        c = srcFile.read();
        bakFile.write(c);
      }
      bakFile.close();
      srcFile.close();
    } else {
      debug(F("**ERROR** creating backup file %s!\n"), backupfile);
      if (srcFile) {
        srcFile.close();
      }
      if (bakFile) {
        bakFile.close();
      }
    }
  }
  // now save file
  debug(F("Save to: %s\n"), srcFileName);
  
 File newFile = openFile(srcFileName, FILE_WRITE);
  if (newFile) {
    char buffer[1600];
    char* p = settingsToString(buffer, true);
    //Serial.println(p);
    newFile.print(p);
    newFile.close();
    //free(p);
    result = true;
    debug(F("Settings saved to %s\n"), srcFileName);
    Serial.println("here1");
  } else {
    debug(F("**ERROR** saving to %s\n"), srcFileName);
  }
  Serial.println("Before return");  
  return result;
  
}

/**
 * Set the specified file as the default profile that 
 * is loaded with TKTalkie starts
 */
boolean setDefaultProfile(char *filename) 
{
    addFileExt(filename);
    debug(F("Setting default profile to %s\n"), filename);
    char profiles[MAX_FILE_COUNT][FILENAME_SIZE];
    int total = listFiles(PROFILES_DIR, profiles, MAX_FILE_COUNT, FILE_EXT, false, false);
    boolean result = false;
    boolean found = false;
    for (int i = 0; i < total; i++) {
      if (strcasecmp(profiles[i], filename) == 0) {
        strlcpy(Config.profile, filename, sizeof(Config.profile));
        found = true;
        break;
      }
    }

    // save results to file if entry was not found
    if (found == true) {
      result = saveConfig();
    } else {
      debug(F("Filename was not an existing profile\n"));
    }  

    if (result == true) {
      debug(F("Default profile set\n"));
    } else {
      debug(F("**ERROR** setting default profile\n"));
    }
  
    return result;
}

/**
 * Remove a profile from the list and delete the file
 */
boolean deleteProfile(char *filename) 
{
  boolean result = false;
  addFileExt(filename);
  char path[SETTING_ENTRY_MAX];
  strcpy(path, PROFILES_DIR);
  strcat(path, filename);
  debug(F("Deleting file %s\n"), path);
  // can't delete current profile
  if (strcasecmp(filename, Settings.file) == 0){
    debug(F("Cannot delete current profile\n"));
    result = false;
  } else {
    result = deleteFile(path);
    // if the profile filename was the default profile, 
    // set the default profile to the currently loaded profile
    if (result == true) {
      if (strcasecmp(filename, Config.profile) == 0) {
        debug(F("Profile was default -> Setting default profile to current profile\n"));
        result = setDefaultProfile(Settings.file);
      }
    }

  }
  return result;
}

/**
 * Load specified settings file
 */
void loadSettings(const char *filename, Settings_t *Settings, boolean apply) 
{

  Serial.println("--- AT READ SETTINGS FILE");
  Serial.println(filename);
  
  const size_t bufferSize = 6*JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(5) + JSON_ARRAY_SIZE(6) + 4*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 3*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + JSON_OBJECT_SIZE(9) + 780;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  char srcFileName[25];
  strcpy(srcFileName, PROFILES_DIR);
  strcat(srcFileName, filename);

  Serial.println("Opening file from SD card");
  
  File file = SD.open(srcFileName);

  if (!file) {
    Serial.println("Error reading file");
  }
  
  JsonObject& root = jsonBuffer.parseObject(file);

  if (!root.success()) {
    Serial.println("ERROR READING SETTINGS FILE!");
  }
  
  file.close();

  strlcpy(Settings->name, root["name"], sizeof(Settings->name)); // "aaaaaaaaaaaaaaaaaaaa"

  JsonObject& volume = root["volume"];

  char buf[30];
    
  Settings->volume.master = volume["master"]; // 0.55
  
  Settings->volume.microphone = volume["microphone"]; // 10
  Settings->volume.linein = volume["linein"]; // 10
  Settings->volume.lineout = volume["lineout"]; // 30

  dtostrf(Settings->volume.master, 0, 3, buf);
  debug(F("Volume.master: %s\n"), buf);
  debug(F("Volume.microphone: %d\n"), Settings->volume.microphone);
  debug(F("Volume.linein: %d\n"), Settings->volume.linein);
  debug(F("Volume.lineout: %d\n"), Settings->volume.lineout);
  
  JsonObject& sounds = root["sounds"];
  strlcpy(Settings->sounds.dir, sounds["dir"], sizeof(Settings->sounds.dir));
  strlcpy(Settings->sounds.start, sounds["start"], sizeof(Settings->sounds.start));
  strlcpy(Settings->sounds.button, sounds["button"], sizeof(Settings->sounds.button)); // "aaaaaaaa.aaa"

  debug(F("Sounds.dir: %s\n"), Settings->sounds.dir);
  debug(F("Sounds.start: %s\n"), Settings->sounds.start);
  debug(F("Sounds.button: %s\n"), Settings->sounds.button);
  
  JsonObject& loop = root["loop"];
  strlcpy(Settings->loop.dir, loop["dir"], sizeof(Settings->loop.dir)); // "/aaaaaaaa/"
  strlcpy(Settings->loop.file, loop["file"], sizeof(Settings->loop.file)); // "aaaaaaaa.aaa"
  Settings->loop.volume = loop["volume"]; // 0.02
  Settings->loop.mute = loop["mute"]; // 1

  debug(F("Loop.dir: %s\n"), Settings->loop.dir);
  debug(F("Loop.file: %s\n"), Settings->loop.file);
  dtostrf(Settings->loop.volume, 0, 3, buf);
  debug(F("Loop.volume %s\n"), buf);
  debug(F("Loop.mute: %d\n"), Settings->loop.mute);
  
  JsonObject& voice = root["voice"];
  Settings->voice.volume = voice["volume"]; // 3
  Settings->voice.dry = voice["dry"]; // 0
  Settings->voice.start = voice["start"]; // 0.043
  Settings->voice.stop = voice["stop"]; // 0.02
  Settings->voice.wait = voice["wait"]; // 1000

  dtostrf(Settings->voice.volume, 0, 3, buf);
  debug(F("Voice.volume: %s\n"), buf);
  dtostrf(Settings->voice.dry, 0, 2, buf);
  debug(F("Voice.dry: %s\n"), buf);
  dtostrf(Settings->voice.start, 0, 4, buf);
  debug(F("Voice.start %s\n"), buf);
  dtostrf(Settings->voice.stop, 0, 4, buf);
  debug(F("Voice.stop: %s\n"), buf);
  debug(F("Voice.wait: %d\n"), Settings->voice.wait);
  
  JsonObject& effects = root["effects"];
  strlcpy(Settings->effects.dir, effects["dir"], sizeof(Settings->effects.dir)); // "/aaaaaaaa/"
  Settings->effects.volume = effects["volume"]; // 1
  Settings->effects.highpass = effects["highpass"]; // 1

  dtostrf(Settings->effects.volume, 0, 3, buf);
  debug(F("Effects.volume: %s\n"), buf);
  debug(F("Effects.highpass: %d\n"), Settings->effects.highpass);
  
  Settings->effects.bitcrusher.bits = effects["bitcrusher"]["bits"]; // 16
  Settings->effects.bitcrusher.rate = effects["bitcrusher"]["rate"]; // 44100

  debug(F("Bitcrusher.bits: %d\n"), Settings->effects.bitcrusher.bits);
  debug(F("Bitcrusher.rate: %d\n"), Settings->effects.bitcrusher.rate);
  
  Settings->effects.chorus.voices = effects["chorus"]["voices"]; // 10
  Settings->effects.chorus.delay = effects["chorus"]["delay"]; // 1000

  debug(F("Chorus.voices: %d\n"), Settings->effects.chorus.voices);
  debug(F("Chorus.delay: %d\n"), Settings->effects.chorus.delay);
  
  JsonObject& effects_flanger = effects["flanger"];
  Settings->effects.flanger.delay = effects_flanger["delay"]; // 32
  Settings->effects.flanger.offset = effects_flanger["offset"]; // 10
  Settings->effects.flanger.depth = effects_flanger["depth"]; // 10
  Settings->effects.flanger.freq = effects_flanger["freq"]; // 0.0625

  debug(F("Flanger.delay: %d\n"), Settings->effects.flanger.delay);
  debug(F("Flanger.offset: %d\n"), Settings->effects.flanger.offset);
  debug(F("Flanger.depth: %d\n"), Settings->effects.flanger.depth);
  dtostrf(Settings->effects.flanger.freq, 0, 4, buf);
  debug(F("Flanger.freq: %s\n"), buf);
  
  Settings->effects.noise = effects["noise"]; // 0.014
  Settings->effects.mute = effects["mute"]; // 1

  dtostrf(Settings->effects.noise, 0, 3, buf);
  debug(F("Noise.volume: %s\n"), buf);
  
  Settings->eq.active = root["eq"]["active"]; // 1
  
  JsonArray& eq_bands = root["eq"]["bands"];
  Settings->eq.bands[0] = eq_bands[0]; // -0.2
  Settings->eq.bands[1] = eq_bands[1]; // -0.4
  Settings->eq.bands[2] = eq_bands[2]; // -0.35
  Settings->eq.bands[3] = eq_bands[3]; // -0.35
  Settings->eq.bands[4] = eq_bands[4]; // -0.35

  Settings->sleep.timer = root["sleep"]["timer"]; // 60000
  strlcpy(Settings->sleep.file, root["sleep"]["file"], sizeof(Settings->sleep.file)); // "aaaaaaaa.aaa"

  strlcpy(Settings->glove.dir, root["glove"]["dir"], sizeof(Settings->glove.dir));
  
  JsonArray& glove_buttons = root["glove"]["buttons"];

  for (byte i = 0; i < 6; i++) {
    strcpy(buf, glove_buttons[i][0]);
    strcat(buf, ";");
    strcat(buf, glove_buttons[i][1]);
    strlcpy(Settings->glove.settings[i], buf, sizeof(Settings->glove.settings[i]));
    memset(buf, 0, sizeof(buf));
  }
  
  // Apply settings
  if (apply == true) {
    setVolume();
    setMicGain();
    setLineout();
    setLinein();
    setHipass();
    setEffectsVolume();
    setNoiseVolume();
    setVoiceVolume();
    setDryVolume();
    setLoopVolume();
    setEq();
    setEqBands();
    setBitcrusher();
    setEffectsDir();
    setSoundsDir();
    setLoopDir();
    setLoopMute();
    setEffectsMute();
    setSleepTimer();
    setChorus();
    setFlanger(); 
    // Configure Buttons
    for (byte i = 0; i < 6; i++) {
      ConfigureButton(i);
    }
  }  
  
}


