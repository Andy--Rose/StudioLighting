#include <boarddefs.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <ir_Lego_PF_BitStreamEncoder.h>

enum Themes { NORMAL, HALLOWEEN, CHRISTMAS };

#define COLUMNS 5
#define CHASE_SECTION_SIZE 5
#define COLOR_INTERVAL 40
#define LAG_OFFSET 2
#define PATTERN_COUNT 7

#define WAVE_FADE_SIZE 15   // Size of the fade trailWaveEnabled

const TProgmemPalette16 themePalette_p PROGMEM;

class StudioLightingPattern
{
  public:    
    // Constructor - calls base-class constructor to initialize strip
    StudioLightingPattern(uint16_t pixels, CRGB* ledAlloc)
    {
      TotalSteps = pixels;
      TotalLeds = pixels;
      leds = ledAlloc;
    }

    void Init(Themes theme, int brightness = 255){
      Serial.println("INIT");
      CalculateIntervals();
      InitTheme(theme);
      delay(500);
      
      ActivePattern = Patterns[iPattern];
      ActiveBrightness = brightness;
      FastLED.setBrightness(ActiveBrightness);
      currentPalette = themePalette;
      delay(500);
      
      changed_time = millis();
      colored_time = millis();
      lastUpdate = millis();
      delay(500);

      SetColorWipe(); // Starting pattern
      
      Update();
    }

    // Update the pattern
    void Update(){
      if (!PatternLocked) { ChangePattern(false); }
      if((millis() - lastUpdate) > UpdateInterval){ // time to update
        lastUpdate = millis();
        if (ActivePattern == "RAINBOW_CYCLE") {
          RainbowCycleUpdate();
        } else if (ActivePattern == "COLOR_WIPE") {
          ColorWipeUpdate();
        } else if (ActivePattern == "WAVE") {
          WaveUpdate();
        } else if (ActivePattern == "THEATER_CHASE") {
          TheaterChaseUpdate();
        } else if (ActivePattern == "PARTY") {
          PartyUpdate();
        } else if (ActivePattern == "SLOW_FADE") {
          SlowFadeUpdate();
        } else {
          // do nothing
        }
      }
    }

    void ClearLights() {
      for (int i=0; i<TotalLeds; i++) {
        SetPixel(i, 0);
      }
      FastLED.show();
    }
    
    void NextPattern() {
      ChangePattern(true);
    }

    void PreviousPattern() {
      GoBackPattern = true;
      ChangePattern(true);
      GoBackPattern = false;
    }

    void SetBPM(int tempo) {
      BPM = tempo;
      CalculateIntervals();
    }

    //**************TEST METHODS****************//
    void LockPattern() {
      PatternLocked = true;
    }

    void UnlockPattern() {
      PatternLocked = false;
    }

    void SetRainbow() {
      Serial.println("Set Rainbow");
      ActivePattern = "RAINBOW_CYCLE";
      iPattern = 0;
      RainbowCycle();
    }

    void SetColorWipe() {
      Serial.println("Set Colorwipe");
      ActivePattern = "COLOR_WIPE";
      iPattern = 1;
      ColorWipe();
    }

    void SetTheaterChase() {
      Serial.println("Set Theater Chase");
      ActivePattern = "THEATER_CHASE";
      iPattern = 2;
      TheaterChase();
    }

    void SetWave() {
      Serial.println("Set Wave");
      ActivePattern = "WAVE";
      iPattern = 3;
      Wave();
    }

    void SetParty() {
      Serial.println("Set Party");
      ActivePattern = "PARTY";
      iPattern = 4;
      Party();
    }

    void SetSlowFade() {
      Serial.println("Set Slow Fade");
      ActivePattern = "SLOW_FADE";
      iPattern = 5;
      SlowFade();
    }

   private:
   //***************VARIABLES***************//
    // Settings
    String Patterns[PATTERN_COUNT] = { "RAINBOW_CYCLE", "COLOR_WIPE", "THEATER_CHASE", "WAVE", "PARTY", "SLOW_FADE" };

    String ActivePattern;
    Themes ActiveTheme;

    CRGB *leds;
    
    uint16_t ActiveBrightness;
    uint16_t TotalSteps = 0;        // total number of steps in the pattern
    uint16_t TotalLeds = 0;
    uint8_t TotalRotations = 1;

    // FastLED settings
    CRGBPalette16 currentPalette;
    CRGBPalette16 themePalette;
    TBlendType    currentBlending;

    // Bool rules
    bool PatternLocked = false;
    bool GoingForward = true;
    bool GoBackPattern = false;

    // Active Patterns
    bool TheaterChaseEnabled = true;
    bool RainbowCycleEnabled = true;
    bool ColorWipeEnabled = true;
    bool WaveEnabled = true;
    bool PartyEnabled = true;
    bool SlowFadeEnabled = true;

    // Indices
    uint16_t Index = 0;
    int iPattern = 0;
    int iColor = 0;     // Index for the current cycle color
    int iRotation = 0; // Index for the current rotation of the complete cycle

    // Pattern Intervals
    // These are overridden when the BPM is changed
    double RainbowInterval = 5;
    double WipeInterval = 50;
    double ChaseInterval = 50;
    double WaveInterval = 50;
    double PartyInterval = 50;
    double SlowFadeInterval = 50;

    // Timing
    int BPM = 128;
    int UpdateInterval = 5;     // milliseconds between updates
    int PatternInterval = 30; // Seconds before changing the pattern
    unsigned long lastUpdate;   // last update of position
    unsigned long this_time = millis();
    unsigned long changed_time = this_time - (PatternInterval * 1000);  // Set to init right away
    unsigned long colored_time = this_time;    
 
    //***************UTILITY METHODS***************//
    // Increment the Index and reset at the end
    void Increment(){
//      Serial.println("Increment: " + (String) Index); 
      if (GoingForward){
        Index++;
        if (Index == TotalSteps){
          Index = TotalSteps - 1;
          PatternComplete(); // call the comlpetion callback
        }
      }
      else { // reverse
        Index--;
        if (Index == 0){
          PatternComplete(); // call the comlpetion callback
        }
      }
    }

    void PatternComplete()
    {
      if (iRotation + 1 < TotalRotations){
           iRotation++;
           if (GoingForward) { Index = 0; } else { Index = TotalSteps; }
      } else {
        iRotation = 0;
//        Serial.println("PatternComplete");
        if (ActivePattern == "COLOR_WIPE") {
          Reverse();
          NextColor();
        } else if (ActivePattern == "THEATER_CHASE"
          || ActivePattern == "WAVE") {
          NextColor();
          Index = 0;
        } else {
          Index = 0;  // Start Over
        }
      }
    }

    // Reverse direction of the pattern
    void Reverse(){
//      Serial.println("Reversing");
      GoingForward = !GoingForward;
    }

    // set intervals based on BPM
    void CalculateIntervals() {
      Serial.println("Calculating Intervals");
      RainbowInterval = (((BPM * 60) / TotalLeds) / 4)  - LAG_OFFSET;
      WipeInterval = (((BPM * 60) / TotalLeds) / 4)  - LAG_OFFSET; //(4 beats)
      ChaseInterval = ((BPM * 60) / TotalLeds) - LAG_OFFSET;
      WaveInterval = (((BPM * 60) / TotalLeds) / 4)  - LAG_OFFSET; //(4 beats)
      PartyInterval = ((BPM * 60) / TotalLeds) - LAG_OFFSET;
      SlowFadeInterval = ((BPM * 60) / TotalLeds) - LAG_OFFSET;
    }

    //***************COLOR METHODS***************//
    void SetPixel(int pixel) {
      leds[pixel] = ColorFromPalette(currentPalette, iColor, ActiveBrightness, currentBlending);
    }

    void SetPixel(int pixel, int brightness) {
      leds[pixel] = ColorFromPalette(currentPalette, iColor, brightness, currentBlending);
    }
    
    void NextColor() {
      iColor += COLOR_INTERVAL;
    }

    void NextColor(int interval) {
      iColor += interval;
    }

    //***************THEME UTILITY METHODS***************//
    // Set the properties and colors based on the theme
    void InitTheme(Themes theme){
      ActiveTheme = theme;
      if (ActiveTheme == NORMAL){
        PatternInterval = 30;
        themePalette = PartyColors_p;
      }
      
      if (ActiveTheme == HALLOWEEN) {
        PatternInterval = 30;

        const TProgmemPalette16 themePalette_p PROGMEM =
        {
            CRGB::Purple,
            CRGB::Orange,
            CRGB::Red,
            CRGB::White,
            CRGB::Purple,
            CRGB::Green
        };

        themePalette = themePalette_p;

        // deactivate undesired modes
        RainbowCycleEnabled = false;   
      }
      
      if (ActiveTheme == CHRISTMAS) {
        PatternInterval = 30;

        const TProgmemPalette16 themePalette_p PROGMEM =
        {
            CRGB::Red,
            CRGB::Green,
            CRGB::White,
            CRGB::Gold,
            CRGB::Red,
            CRGB::Green
        };

        themePalette = themePalette_p;

        // deactivate undesired modes
        RainbowCycleEnabled = false;
      }
    }

    //***************PATTERN UTILITY METHODS***************//
    void ChangePattern(bool force) {
      this_time = millis();
      if((this_time - changed_time) > (PatternInterval * 1000) || force) {
        GoingForward = true;
        currentPalette = themePalette;
        changed_time = millis();
        SetNextPatternIndex();
        ActivePattern = Patterns[iPattern];
        Serial.println(ActivePattern);
        if (ActivePattern == "RAINBOW_CYCLE") {
          RainbowCycle();
        } else if (ActivePattern == "COLOR_WIPE") {
          ColorWipe();
        } else if (ActivePattern == "WAVE") {
          Wave();
        } else if (ActivePattern == "THEATER_CHASE") {
          TheaterChase();
        } else if (ActivePattern == "PARTY") {
          Party();
        } else if (ActivePattern == "SLOW_FADE") {
          SlowFade();
        } else {
          ActivePattern = "RAINBOW_CYCLE";
          iPattern = 0;
          RainbowCycle();
        }
      }
    }

    void SetNextPatternIndex() {
      Serial.println(PATTERN_COUNT);
      if (GoBackPattern) {
        if (iPattern == 0) { 
          iPattern = (PATTERN_COUNT - 1); 
        }
        else {
          iPattern--; 
        }
      } 
      else {
        if (iPattern == (PATTERN_COUNT - 1)) {
          iPattern = (int) 0; 
        }
        else { iPattern = iPattern + 1; }
      }
      Serial.println((String)iPattern);
      if (!IsEnabledPattern(Patterns[iPattern])){
        SetNextPatternIndex();
      }
    }

    bool IsEnabledPattern(String pattern) {
      if (pattern == "RAINBOW_CYCLE") {
        return RainbowCycleEnabled;
      } else if (pattern == "COLOR_WIPE") {
        return ColorWipeEnabled;
      } else if (pattern == "WAVE") {
        return WaveEnabled;
      } else if (pattern == "THEATER_CHASE") {
        return TheaterChaseEnabled;
      } else if (pattern == "PARTY") {
        return PartyEnabled;
      } else if (pattern == "SLOW_FADE") {
        return SlowFadeEnabled;
      } else { 
        return true;
      }
    }

    //***************PATTERN METHODS***************//
    void RainbowCycle(){
      Serial.println("Begin RAINBOW_CYCLE");
      currentPalette = RainbowColors_p;
      currentBlending = LINEARBLEND;
      UpdateInterval = RainbowInterval;
      TotalRotations = 1;
      TotalSteps = 255;
      Index = 0;
    }

    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate(){
      static int rainbowIndex = 1;
      static int rainbowColorInterval = 255 / TotalLeds;
      for(int i=0; i<TotalLeds; i++){
        leds[i] = ColorFromPalette(currentPalette, rainbowIndex, ActiveBrightness, currentBlending);
        rainbowIndex += rainbowColorInterval;
      }
      FastLED.show();
    }
    
    // Initialize for a ColorWipe
    void ColorWipe(){
      Serial.println("Begin COLOR_WIPE");
      currentPalette = themePalette;
      currentBlending = NOBLEND;
      UpdateInterval = WipeInterval;
      TotalRotations = 1;
      TotalSteps = TotalLeds;
      Index = 0;
    }

    // Update the Color Wipe Pattern
    void ColorWipeUpdate(){
      SetPixel(Index);
      FastLED.show();
      Increment();
    }

    // Initialize for a Theater Chase
    void TheaterChase(){
      Serial.println("Begin THEATER_CHASE");
      currentBlending = NOBLEND;
      currentPalette = themePalette;
      UpdateInterval = ChaseInterval;
      TotalRotations = 1;
      TotalSteps = TotalLeds;
      Index = 0;
    }

    // Update the Theater Chase Pattern
    void TheaterChaseUpdate(){
      for(int i=0; i<TotalLeds; i++){
        if ((i + Index) % CHASE_SECTION_SIZE == 0){
          leds[i] = ColorFromPalette(currentPalette, iColor, ActiveBrightness, currentBlending);
        }
        else{
          leds[i] = ColorFromPalette(currentPalette, iColor + 40, ActiveBrightness, currentBlending);
        }
      }
      FastLED.show();
      Increment();
    }

    void Wave()
    {
      Serial.println("Begin WAVE");
      UpdateInterval = WaveInterval;
      TotalRotations = 4;
      TotalSteps = TotalLeds;
      Index = 0;
    }

    // Update the Wave Pattern
    void WaveUpdate()
    {
        ClearLights();
        SetPixel(Index);

        for (int i=0; i<WAVE_FADE_SIZE; i++){
          int brightness = (255 / (float)WAVE_FADE_SIZE) * (WAVE_FADE_SIZE-i);
          
          // Back
          int pointBack = Index - i;
          if (pointBack < 0) { pointBack = TotalSteps + pointBack; }
          SetPixel(pointBack, brightness);

          // Front
          int pointFront = Index + i;
          if (pointFront > TotalSteps) { pointFront = pointFront - TotalSteps; }
          SetPixel(pointFront, brightness);
        }

        FastLED.show();
        Increment();
    }

    // Initialize for a Theater Chase
    void Party(){
      Serial.println("Begin PARTY");
      currentPalette = RainbowColors_p;
      UpdateInterval = PartyInterval;
      TotalRotations = 1;
      TotalSteps = 1;
      Index = 0;
    }

    // Update the Theater Chase Pattern
    void PartyUpdate(){
      for(int i=0; i<TotalLeds; i++){
        SetPixel(i);
        NextColor(30);
      }
      FastLED.show();
      Increment();
    }

    // Initialize for a ColorWipe
    void SlowFade(){
      Serial.println("Begin COLOR_WIPE");
      currentPalette = RainbowColors_p;
      currentBlending = LINEARBLEND;
      UpdateInterval = SlowFadeInterval;
      TotalRotations = 1;
      TotalSteps = 255;
      Index = 0;
    }

    // Update the Color Wipe Pattern
    void SlowFadeUpdate(){
      for(int i=0; i<TotalLeds; i++){
        leds[i] = ColorFromPalette(currentPalette, iColor, ActiveBrightness, currentBlending);
      }
      NextColor(1);
      FastLED.show();
      Increment();
    }
};

