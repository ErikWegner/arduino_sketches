send_status = " "
send_reset_delay = 0
sleep_delay = 0

config_loaded = false
wifi_counter = 0

base = -225603
scale = 21.49584
lastweightvalue = 0.0
lastweightcounter = 0

API_URL = nil
API_TOKEN = nil

led_pin = 0 -- onboard LED (red)
gpio.mode(led_pin, gpio.OUTPUT)

function setupDisplay()
    local sda = 2
    local scl = 1
    i2c.setup(0, sda, scl, i2c.SLOW)
    local sla = 0x3c
    disp = u8g.sh1106_128x64_i2c(sla)
end

function showText(text)
    disp:setFont(u8g.font_6x10)
    disp:setFontRefHeightExtendedText()
    disp:setDefaultForegroundColor()
    disp:setFontPosTop()
    disp:firstPage()
    repeat
        disp:drawStr( 0, 0, text);
    until disp:nextPage() == false
end

function start()
    setupDisplay()
    showText('Start')
    if file.exists("device.config.lua") then
        dofile("device.config.lua")
        hx711.init(5,6)
        showText('Geladen')
        station_cfg={}
        station_cfg.ssid = WIFI_SSID
        station_cfg.pwd = WIFI_PASSWORD
        wifi.setmode(wifi.STATION)
        wifi.sta.config(station_cfg)
        wifi.sta.connect()
        config_loaded = true
    else
        showText(0, 'Konfiguration fehlerhaft')
    end
end

function loopfunc()
    if not config_loaded then return end
    local wifi_status
    local led_status
    if wifi.sta.getip ( ) == nil then
        if wifi_counter % 2 == 0 then
            led_status = gpio.HIGH
            wifi_status = "W . "
        else
            led_status = gpio.LOW
            wifi_status = "W : "
        end
        wifi_status = (wifi_counter % 2 == 0) and "W . " or "W : "
        wifi_counter = wifi_counter + 1
    else
        wifi_status = "W ^ "
        led_status = gpio.LOW -- turn on
    end

    local raw_data = hx711.read(0)
    local weightvalue = math.floor((raw_data - base) / scale / 100)/10
    if weightvalue <= 0.1 then
        weightvalue = 0.0
        sleep_delay = sleep_delay + 1
    end
    
    if lastweightvalue > (weightvalue - 0.2) and lastweightvalue < (weightvalue + 0.2) then
        if lastweightcounter < 5 and lastweightvalue > 0 then
            lastweightcounter = lastweightcounter + 1
            sleep_delay = 0
        end        
    else
        lastweightcounter = 0
        lastweightvalue = weightvalue
    end

    if lastweightcounter >= 5 and send_reset_delay == 0 and send_status == " " then
        if wifi_status == "W ^ " then
            send_status = "->"
            http.post(
                API_URL,
                'Content-Type: application/json\r\nAuthorization: Token ' .. API_TOKEN .. '\r\n',
                '{"value":' .. tostring(weightvalue) .. '}',
                function(code, data)
                    if (code < 0 or code >= 400) then
                        send_status = "Err " .. tostring(code)
                        print("HTTP request failed")
                        print(data)
                    else
                        send_status = "Gespeichert"
                        send_reset_delay = 10
                    end
                end
            )
        end
    end

    if send_reset_delay > 0 then
        send_reset_delay = send_reset_delay - 1
        if send_reset_delay == 0 then
            send_status = " "
        end
    end

    local statusline = wifi_status .. '  ' .. tostring(lastweightcounter) .. '  ' .. send_status
    if sleep_delay > 20 then
        statusline = ' ---  o f f  ---'
        led_status = gpio.HIGH -- turn off
    end

    gpio.write(led_pin, led_status)
    disp:firstPage()
    repeat
        disp:setFont(u8g.font_6x10)        
        disp:drawStr( 0, 8, statusline);

        disp:setFont(u8g.font_gdb25n);
        disp:drawStr(0, 63, string.format("%3.1f", weightvalue))
    until disp:nextPage() == false

    if sleep_delay > 20 then
        node.dsleep(0)
    end
    
end

if tmr.create():alarm(250, tmr.ALARM_SINGLE, start)
then
    tmr.create():alarm(1000, tmr.ALARM_AUTO, loopfunc)
end
