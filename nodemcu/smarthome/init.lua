T = 0.0
B = 0.0
statustext = ""
m = nil
hasWifi = false
hasMqtt = false
tls.cert.verify([[
-----BEGIN CERTIFICATE-----
MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/
MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT
DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow
PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD
Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB
AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O
rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq
OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b
xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw
7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD
aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV
HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG
SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69
ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr
AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz
R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5
JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo
Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ
-----END CERTIFICATE-----
]])
tls.cert.verify(false)

function u8g2_prepare()
  disp:setFont(u8g2.font_6x10_tf)
  disp:setFontRefHeightExtendedText()
  disp:setDrawColor(1)
  disp:setFontPosTop()
  disp:setFontDirection(0)
end

function init_peripherals() 
    local id, sda, scl = 0, 3, 4
    i2c.setup(id, sda, scl, i2c.SLOW) -- call i2c.setup() only once
    bme280.setup()
    disp = u8g2.ssd1306_i2c_128x64_noname(id, 0x3C)
    u8g2_prepare()
end

function showText(text)
    statustext = text
    print(text)
    --displayRefresh()
end

function readAndDrawTempBaro()
--    T, B = bme280.read()
end

function startBme280Timer()
    local mytimer = tmr.create()
    mytimer:register(10000, tmr.ALARM_AUTO, readAndDrawTempBaro)
    mytimer:start()
    readAndDrawTempBaro()
end

function displayRefresh()
    disp:clearBuffer()
    disp:drawStr( 0, 0, statustext)
    disp:drawStr( 0, 10, string.format("%2.2f", T/100))
    disp:drawStr( 0, 20, string.format("%2.3f", B/1000))
    disp:sendBuffer()
end

function init_displayRefresh()
    local mytimer = tmr.create()
    mytimer:register(5000, tmr.ALARM_AUTO, displayRefresh)
    mytimer:start()
end

function handle_mqtt_error(client, reason)
    hasMqtt = false
    showText("Disconnected")
    print("failed reason: " .. reason)
    tmr.create():alarm(10 * 1000, tmr.ALARM_SINGLE, do_mqtt_connect)
end

function do_mqtt_connect()
    hasMqtt = false
    showText('WiFi - MQTT conn ...')
    print("Connecting to " .. MQTT_SERVERNAME)
    m:connect(MQTT_SERVERNAME, MQTT_PORT, MQTT_SECURE, function(client) 
        showText("connected")
        hasMqtt = true
    end, handle_mqtt_error)
end

function connected()
    do_mqtt_connect()
end

function wifiEvents()
    wifi.eventmon.register(wifi.eventmon.STA_CONNECTED, function(e)
        showText('Waiting for IP')
        print("\n\tSTA - CONNECTED".."\n\tSSID: "..e.SSID.."\n\tBSSID: "..
        e.BSSID.."\n\tChannel: "..e.channel)
    end)
    wifi.eventmon.register(wifi.eventmon.STA_DISCONNECTED, function(e)
        showText('Offline')
        hasWifi = false
        print("\n\tSTA - DISCONNECTED".."\n\tSSID: "..e.SSID.."\n\tBSSID: "..
        e.BSSID.."\n\treason: "..e.reason)
    end)
     wifi.eventmon.register(wifi.eventmon.STA_GOT_IP, function(e)
        showText(e.IP)
        hasWifi = true
        print("\n\tSTA - GOT IP".."\n\tStation IP: "..e.IP.."\n\tSubnet mask: "..
        e.netmask.."\n\tGateway IP: "..e.gateway)
        connected()
    end)
    wifi.eventmon.register(wifi.eventmon.STA_DHCP_TIMEOUT, function()
        showText('DHCP timeout')
        print("\n\tSTA - DHCP TIMEOUT")
    end)
end

function go_main()
    wifiEvents()
    --init_peripherals()
    --init_displayRefresh()
    showText('I N I T')
    if file.exists("device.config.lua") then
        dofile("device.config.lua")
        m = mqtt.Client("r1", 60, MQTT_USERNAME, MQTT_PASSWORD)
        m:on("connect", function(client) print ("mqtt connected") end)
        m:on("offline", function(client) print ("mqtt offline") end)
        showText('Connecting...')
        station_cfg={}
        station_cfg.ssid = WIFI_SSID
        station_cfg.pwd = WIFI_PASSWORD
        station_cfg.auto = true
        station_cfg.save = false
        wifi.setmode(wifi.STATION)
        wifi.sta.sethostname("NodeMCU-R1")
        wifi.sta.config(station_cfg)
        --wifi.sta.connect()
    else
        showText('Check config')
    end
    --startBme280Timer()
end

go_main()
