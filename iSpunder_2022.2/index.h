// index.h

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
</head>

<body bgcolor="#1210CB">
<h1>WLan Config Manager</h1>
<form action='/wifi' method='POST'>
<p>SSID:<br>
<input name="tf_ssid" type="text" required="required" id="tf_ssid" size="50">
</p>
<p>PSK :<br>
<input name="tf_psk" type="text" id="tf_psk" size="50">
</p>
<p>&nbsp;</p>
<p>
<button type='submit' name='Speichern' value='speichern'>Speichern</button></form>
</p>
<p>&nbsp;</p>
<form action='' method='post'>
<button type='submit' name='AP_OFF' value='apoff'>Accesspoint OFF</button></form>
</body>
</html>
)=====";

const char Spunder_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
</head>

<body bgcolor="#DAF612">
<h1>Spunder Manager</h1>
<form action='/spunder' method='POST'>
<p>SpunderNummer 01 - 99:<br>
<input name="tf_spundernr" type="text" id="tf_spundernr" size="25">
</p>
<p>SudNummer 0 - 999:<br>
<input name="tf_sudnr" type="text" required="required" id="tf_sudnr" size="25">
</p>
<p>&nbsp;</p>
<p>
<button type='submit' name='Speichern' value='speichern'>Speichern</button></form>
</p>
<p>&nbsp;</p>
</body>
</html>
)=====";
