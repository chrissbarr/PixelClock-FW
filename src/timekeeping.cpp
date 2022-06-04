#include "timekeeping.h"

std::unique_ptr<RTC_DS3231> rtc;

bool initialiseTime()
{
  if (initialiseRTC()) {
    // Set Time to sync from RTC
    setSyncProvider([](){ return time_t(rtc->now().unixtime()); });
    setSyncInterval(60);

    if(timeStatus() != timeSet) {
      Serial.println("Unable to sync with the RTC.");
    } else {
      Serial.println("RTC has set the system time"); 
    }
  } else {
    Serial.println("Setting time to placeholder value.");
    setTime(11,55,50,1,1,2022);
  }
  return true;
}

bool initialiseRTC()
{
  Serial.print("Initialising RTC: ");

  rtc = std::make_unique<RTC_DS3231>();

  if (!rtc->begin()) {
    Serial.println("Error!");
    return false;
  } else {
    Serial.println("Success!");

    if (rtc->lostPower()) {
      Serial.println("RTC has lost power and time needs to be set!");
    } else {
      Serial.println("RTC reports it has not lost power.");
    }

    DateTime now = rtc->now();
    Serial.println("RTC has time: ");
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");
    return true;
  }
}

ClockFaceTimeStruct timeCallbackFunction(time_t time)
{
  // extract elements of time into struct
  ClockFaceTimeStruct val;
  val.hour12 = hourFormat12(time);
  val.hour24 = hour(time);
  val.minute = minute(time);
  val.second = second(time);
  return val;
}

ClockFaceTimeStruct timeCallbackFunction()
{
  return timeCallbackFunction(now());
}

void setTimeGlobally(uint32_t timeToSet)
{
  Serial.println("Setting time...");
  if (rtc) { 
    Serial.println("Setting RTC time...");
    rtc->adjust(timeToSet); 
  }
  setTime(timeToSet);
}

LoopTimeManager::LoopTimeManager(uint32_t desiredLoopDuration, uint32_t statReportInterval) :
  desiredLoopDuration(desiredLoopDuration), statReportInterval(statReportInterval)
{
  loopTimeMin = std::numeric_limits<uint16_t>::max();
  loopTimeMax = 0;
}

void LoopTimeManager::idle()
{
  // Manage loop timing
  uint32_t loopTime = millis() - lastLoopTime;

  loopTimeAvg = approxRollingAverage(loopTimeAvg, float(loopTime), 1000);
  if (loopTime > loopTimeMax) { loopTimeMax = loopTime; }
  if (loopTime < loopTimeMin) { loopTimeMin = loopTime; }

  if (millis() - lastStatReportTime > statReportInterval) {
    // print timing stats
    Serial.println("Loop Timing Statistics");
    Serial.print("Avg time:" ); Serial.println(loopTimeAvg);
    Serial.print("Min time:" ); Serial.println(loopTimeMin);
    Serial.print("Max time:" ); Serial.println(loopTimeMax);
    //Serial.print("FastLED FPS:" ); Serial.println(FastLED.getFPS());

    // print heap stats
    float usedHeapPercentage = 100 * (float(ESP.getHeapSize() - ESP.getFreeHeap()) / ESP.getHeapSize());
    Serial.print("Heap (free / total / used / minfree / maxalloc): "); 
    Serial.print(ESP.getFreeHeap()); 
    Serial.print(" / "); 
    Serial.print(ESP.getHeapSize()); 
    Serial.print(" / "); 
    Serial.print(usedHeapPercentage);
    Serial.print("% / ");
    Serial.print(ESP.getMinFreeHeap());
    Serial.print(" / ");
    Serial.println(ESP.getMaxAllocHeap());

    // print psram stats
    float usedPsramPercentage = 100 * (float(ESP.getPsramSize() - ESP.getFreePsram()) / ESP.getPsramSize());
    Serial.print("PSRAM (free / total / used / minfree / maxalloc): "); 
    Serial.print(ESP.getFreePsram()); 
    Serial.print(" / "); 
    Serial.print(ESP.getPsramSize()); 
    Serial.print(" / "); 
    Serial.print(usedPsramPercentage);
    Serial.print("% / ");
    Serial.print(ESP.getMinFreePsram());
    Serial.print(" / ");
    Serial.println(ESP.getMaxAllocPsram());

    lastStatReportTime = millis();
    loopTimeMin = std::numeric_limits<uint16_t>::max();
    loopTimeMax = 0;
  }

  // ensure we yield at least once
  yield();
  while (millis() - lastLoopTime < desiredLoopDuration) {
    // idle in this loop until desiredLoopDuration has elapsed
    yield();
  }
  lastLoopTime = millis();
}

constexpr float LoopTimeManager::approxRollingAverage(float avg, float newSample, int N) const
{
  avg -= avg / N;
  avg += newSample / N;
  return avg;
}

