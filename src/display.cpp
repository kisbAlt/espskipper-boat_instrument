#include <Arduino.h>
#include <string.h>
#include <display.h>

#include <Preferences.h>

#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

Preferences preferences;
const StringTranslations eng_strings = {"AVG", "COURSE", "MAX", "DISTANCE", "KM/H", "KNOTS", "Satellites", "TEMP", "DEPTH", "Kilometers", "Celsius", "Meters", "Degrees"};
const StringTranslations hu_strings = {"ÁTLAG", "IRÁNY", "MAXIMUM", "TÁVOLSÁG", "KM/H", "CSOMÓ", "muhold", "HÕFOK", "MÉLYSÉG", "Kilóméter", "Celsius", "Méter", "Fok"};
const uint8_t TIMEZONE_SHIFT = 2;


DisplayHandler::DisplayHandler() : display1(U8G2_R0, /* clk=*/18, /* data=*/23, /* cs=*/5, /* reset=*/22), display2(U8G2_R0, /* clk=*/19, /* data=*/21, /* cs=*/22, /* reset=*/22)
{
    Serial.println("Constructor");
}

void DisplayHandler::Init()
{
    dispSettings.LoadData();
    display1.begin();
    display1.enableUTF8Print();

    display2.begin();
    display2.enableUTF8Print();
    PrepareDraw();
}

void DisplayHandler::PrepareDraw()
{
    display1.clearBuffer();
    display1.setFont(u8g2_font_6x10_tf);
    display1.setFontRefHeightExtendedText();
    display1.setDrawColor(1);
    display1.setFontPosTop();
    display1.setFontDirection(0);

    display2.clearBuffer();
    display2.setFont(u8g2_font_6x10_tf);
    display2.setFontRefHeightExtendedText();
    display2.setDrawColor(1);
    display2.setFontPosTop();
    display2.setFontDirection(0);
}

char maxBuf[10];
char avgBuf[10];
char distBuf[10];
char courseBuf[4];
char depthBuf[4];
char tempBuf[4];
void DisplayHandler::DrawDisplay2(u_int32_t satellites, BoatStats stats, DisplaySettings displaySettings)
{
    StringTranslations texts = getLangTranslations();

    char *speedUnitText;
    if (dispSettings.useKnots)
    {
        dtostrf(stats.maxSpeedKt, 3, 1, maxBuf);
        dtostrf(stats.avgSpeedKt, 3, 1, avgBuf);
        speedUnitText = texts.Knots;
    }
    else
    {
        dtostrf(stats.maxSpeedKmph, 3, 1, maxBuf);
        dtostrf(stats.avgSpeedKmph, 3, 1, avgBuf);
        speedUnitText = texts.Kmph;
    }
    dtostrf(stats.distance, 3, 1, distBuf);
    dtostrf(0, 3, 1, tempBuf);
    dtostrf(0, 3, 1, depthBuf);

    snprintf(courseBuf, sizeof(courseBuf), "%03d", stats.lastCourse);

    display2.clearBuffer();

    if (display2State == SUMMARY)
    {
        DrawSummary();
    }
    else
    {
        char *titleText;
        char *unitText;
        char *drawBuf;
        if (display2State == COURSE)
        {
            titleText = texts.Course;
            drawBuf = courseBuf;
            unitText = texts.Degrees;
        }else if(display2State == DEPTH) {
            titleText = texts.Depth;
            drawBuf = depthBuf;
            unitText = texts.Meters;
        }else if(display2State == TEMP) {
            titleText = texts.WaterTemp;
            drawBuf = tempBuf;
            unitText = texts.Celsius;
        }else if(display2State == SPEED_AVG) {
            titleText = texts.AvgSpeed;
            drawBuf = avgBuf;
            unitText = speedUnitText;
        }else if(display2State == SPEED_MAX) {
            titleText = texts.MaxSpeed;
            drawBuf = maxBuf;
            unitText = speedUnitText;
        }else if(display2State == DISTANCE) {
            titleText = texts.Distance;
            drawBuf = distBuf;
            unitText = texts.Kilometers;
        }
        display1.setFont(u8g2_font_spleen32x64_mn);
        int textWidth = display1.getStrWidth(drawBuf);
        int x = (128 - textWidth) / 2;
        display1.drawStr(x, -1, drawBuf);

        display1.setFont(u8g2_font_6x12_t_symbols);

        textWidth = display1.getStrWidth(titleText);
        x = (128 - textWidth) / 2;
        display1.drawUTF8(x, 54, titleText);

        textWidth = display1.getStrWidth(unitText);
        x = (128 - textWidth) / 2;
    
        display1.drawUTF8(x, 0, unitText);
        
    }

    display2.sendBuffer();
}

void DisplayHandler::DrawSummary()
{
    StringTranslations texts = getLangTranslations();
    display2.setFont(u8g2_font_5x7_tf);
    DrawUIBox();
    display2.drawUTF8(0, 0, texts.AvgSpeed);
    display2.drawUTF8(44, 0, texts.Course);
    display2.drawUTF8(86, 0, texts.MaxSpeed);

    display2.drawUTF8(0, 32, texts.Distance);
    display2.drawUTF8(44, 32, texts.WaterTemp);
    display2.drawUTF8(86, 32, texts.Depth);

    display2.setFont(u8g2_font_logisoso16_tn);

    // drawing buffers to screen
    display2.drawStr(0, 10, avgBuf);
    display2.drawStr(44, 10, courseBuf);
    display2.drawStr(86, 10, maxBuf);

    display2.drawStr(0, 42, distBuf);
    display2.drawStr(44, 42, tempBuf);
    display2.drawStr(86, 42, depthBuf);
}

void DisplayHandler::DrawUIBox()
{
    display2.drawLine(42, 0, 42, 64);
    display2.drawLine(84, 0, 84, 64);

    display2.drawLine(0, 31, 128, 31);
}


void DisplayHandler::DrawDisplay1(BoatStats stats, u_int32_t satellites)
{
    double speed;

    const char *speedFormat;
    if (dispSettings.useKnots)
    {
        speed = stats.lastSpeedKt;
        speedFormat = getLangTranslations().Knots;
    }
    else
    {
        speed = stats.lastSpeedKmph;
        speedFormat = getLangTranslations().Kmph;
    }

    if (speed >= 10)
    {
        dtostrf(speed, 3, 1, speedBuffer);
    }
    else
    {
        dtostrf(speed, 3, 2, speedBuffer);
    }

    // reading ESP32 CPU temp
    cpu_temp_celsius = round(temperatureRead());
    sprintf(satsBuf, "S%d 192.168.4.1 %d/%d %dC", satellites, dispSettings.fullRefreshTime / 1000, dispSettings.speedRefreshTime / 1000, cpu_temp_celsius);

    sprintf(timeBuf, "%02d:%02d", (stats.lastHour+TIMEZONE_SHIFT)%24, stats.lastMinute);

    display1.clearBuffer();
    display1.setFont(u8g2_font_spleen32x64_mn);
    display1.drawStr(0, 0, speedBuffer);
    display1.setFont(u8g2_font_5x7_tf);
    display1.drawStr(0, 56, satsBuf);
    display1.drawStr(0, 0, timeBuf);

    display1.setFont(u8g2_font_6x12_t_symbols);
    int textWidth = display1.getStrWidth(speedFormat);
    int x = (128 - textWidth) / 2;
    display1.drawStr(x, 0, speedFormat);
    
    display1.sendBuffer();
}

StringTranslations DisplayHandler::getLangTranslations()
{
    if (dispSettings.language == ENGLISH)
    {
        return eng_strings;
    }
    else if (dispSettings.language == HUNGARIAN)
    {
        return hu_strings;
    }
    else
    {
        return eng_strings;
    }
}

// Click counts, -1 value is long press
void DisplayHandler::HandleButtonInput(int clickCount)
{
    if (clickCount == 1)
    {
        display2State = static_cast<DisplayState>((static_cast<int>(display2State) + 1) % (maxDisplayState + 1));
    }
}

void DisplaySettings::SaveData()
{
    Serial.println("saving settings...");
    preferences.begin("disp", false);

    preferences.putBool("useKnots", useKnots);
    preferences.putUInt("refresh1", fullRefreshTime);
    preferences.putUInt("refresh2", speedRefreshTime);
    preferences.putInt("language", static_cast<int>(language));

    preferences.end();
}

void DisplaySettings::LoadData()
{
    Serial.println("loading settings...");
    preferences.begin("disp", false);

    if (preferences.isKey("useKnots"))
    {
        useKnots = preferences.getBool("useKnots", false);
    }
    if (preferences.isKey("refresh1"))
    {
        fullRefreshTime = preferences.getUInt("refresh1", false);
        Serial.println("fullrefresh: ");
        Serial.println(fullRefreshTime);
    }
    if (preferences.isKey("refresh2"))
    {
        speedRefreshTime = preferences.getUInt("refresh2", false);
    }
    if (preferences.isKey("language"))
    {
        language = static_cast<Language>(preferences.getInt("language", ENGLISH));
    }

    preferences.end();
}
