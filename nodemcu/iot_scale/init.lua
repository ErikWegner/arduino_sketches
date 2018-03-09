function clr( )
   uart.write (1, string.char(254) .. string.char(128) .. "                                " .. string.char(254) .. string.char(128))
end

statustext = ""
statusclear = 0
statuswifi = " "
statusmqtt = " "
weightvalue = 0.0
lastweightvalue = 0.0
lastweightcounter = 0
deepsleepdelaystartvalue = 20
deepsleepdelay = deepsleepdelaystartvalue
MQTT_CLIENTID = nil
MQTT_HOST = nil
MQTT_PORT = nil
MQTT_USER = nil
MQTT_PASSWORD = nil

function weight(value)
    weightvalue = value or 0.0
    refreshDisplay()
    if lastweightvalue > (weightvalue - 0.2) and lastweightvalue < (weightvalue + 0.2) then
        lastweightcounter = lastweightcounter + 1
    else
        lastweightcounter = 0
        lastweightvalue = weightvalue
    end
    if lastweightcounter == 5 then
        send_weight(weightvalue)
    end
end

function status(text)
    statusclear = 4
    statustext = text
    refreshDisplay()
end

function refreshDisplay()
    if statusclear > 0 then
        statusclear = statusclear - 1
    end
    if statusclear == 1 then
        statustext = ""
    end
    -- Clear and write status text
    uart.write (1, string.char(254) .. string.char(1) .. statustext)
    -- Go to line 2
    uart.write(1, string.char(254) .. string.char(0xc0)  .. string.format("%5.1f", weightvalue) .. " kg   " .. statuswifi .. " " .. statusmqtt)
end

function check_sleep(value)
    if value > 0.0 then
        deepsleepdelay = deepsleepdelaystartvalue
        return
    end

    deepsleepdelay = deepsleepdelay - 1
    status("Aus in " .. tostring(deepsleepdelay))
    if deepsleepdelay < 1 then
        status("Ausschalten")
        --node.dsleep(0, 4)
    end
end

function read_config()
    if file.exists("device.config.lua") then
        status("Lade Konfiguration ...")
        dofile("device.config.lua")
        start_scale()
        start_wifi(WIFI_SSID, WIFI_PASSWORD)
    else
        status("Keine Konfiguration")
    end
end

function start_scale()
    hx711.init(5,6)
    raw_data = hx711.read(0)
    base = -225603
    scale = 21.49584

    tmr.alarm(2, 1000, tmr.ALARM_AUTO, function () 
        raw_data = hx711.read(0)
        value = math.floor((raw_data - base) / scale / 100)/10
        if value < 0 then
            value = 0.0
        end
        weight(value)
        check_sleep(value)
    end)
end

function wifi_connected()
    statuswifi = "T"
end

function wait_for_wifi_conn ( )
   local waits = 1;
   tmr.alarm (1, 1000, tmr.ALARM_AUTO, function ( )
      if wifi.sta.getip ( ) == nil then
         statuswifi = (waits % 2 == 0) and "." or ":"
         waits = waits + 1
      else
         --tmr.unregister (1)
         wifi_connected()
      end
   end)
end

function start_wifi(ssid, wifipwd)
    status("Verbinde " .. ssid)
    station_cfg={}
    station_cfg.ssid = ssid
    station_cfg.pwd = wifipwd
    wifi.setmode(wifi.STATION)
    wifi.sta.config(station_cfg)
    wifi.sta.connect()
    wait_for_wifi_conn()
end

function send_weight(value)
    if statuswifi == "T" then
        send_weight_retry(value, 3)
    end    
end

function send_weight_retry(value, send_retries)
    --dofile("device.config.lua")
    statusmqtt = "."
    status("Wird verbunden")
    
    m = mqtt.Client(MQTT_CLIENTID, 120, MQTT_USER, MQTT_PASSWORD)    
    m:connect(MQTT_HOST, MQTT_PORT, 1,
        -- connected
        function(client)
            statusmqtt = "S"
            status("Wird gesendet")
            -- publish a message with data = hello, QoS = 0, retain = 0
            client:publish(
                "/iot/scale", tostring(weightvalue), 0, 0,
                function(client)
                    status("Gesendet")
                    tmr.alarm(5, 1500, tmr.ALARM_SINGLE, function()
                        statusmqtt = " "
                        status("")
                    end)

                    client:close();
                end)
        end,
        -- connection error
        function(client, reason)
            statusmqtt = "E"
            status(reason)
            client:close();
            
            -- try again?
            if send_retries > 0 then
                send_weight_retry(value, send_retries - 1)
            end
        end)

end

tmr.alarm(5, 250, tmr.ALARM_SINGLE, function()
    uart.setup(1, 9600, 8, uart.PARITY_NONE, uart.STOPBITS_1)
    status("Start")
    read_config()
end)
