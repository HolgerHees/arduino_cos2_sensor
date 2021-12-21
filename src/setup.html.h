const char SETUP_PAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Sensor Setup</title>
    <script type="text/javascript" language="JavaScript" src="data.js"></script>
    <style>
        body{
            font-family: 'Open Sans', sans-serif;
            background-color: #fff;
        }

        #container{
            border: 1px solid #d2d2d2;
            max-width: 600px; 
            vertical-align: middle; 
            margin-left: auto; 
            margin-right: auto;
            padding: 5px;
            text-align: center;
        }
        
        div.table {
          display: table;
        }
        div.tr {
          display: table-row;
          position: relative;
        }
        div.tr div {
          display: table-cell;
          text-align: left;
        }
        div.tr div:first-child {
          font-weight: bold;
        }
        div.tr.header {
          height: 3em;
        }
        div.tr.header div:first-child {
          background-color: #fff;
          position: absolute;
          z-index: 1;
          top: 0;
          bottom: 0;
          left: 20px;
          padding: 5px;
          font-size: 1.5em;
        }
        div.tr.header div:last-child {
          position: absolute;
          z-index: 0;
          left: 0;
          right: 0;
          bottom: 0;
          top: 0;
        }
        div.tr.header div:last-child hr {
          margin-top: 1.5em;
        }
        div.tr.footer div:last-child {
          text-align: right;
        }
        input {
            border-radius: 3px;
            color: #555555;
            border: 1px solid #cccccc;
            display: inline-block;
            margin: 5px 5px 5px;
            padding: 4px;
            font-size: 1em;
        }
        input:focus, textarea:focus
        {
            border-color: rgba(82, 168, 236, 0.8);
            box-shadow: inset 0 1px 1px rgba(0, 0, 0, 0.075),0 0 8px rgba(82, 168, 236, 0.6);
            outline: 0;
        }
        #successMsg {
            color: green; 
            font-weight: bold;
        }
    </style>
  </head>
  <body>
  <div id="container">
    <h2>Sensor Setup</h2>
    <form method="post" action="submit">
    <div class="table">
      <div class="tr header">
        <div>Wifi network</div>
        <div><hr/></div>
      </div>
      <div class="tr">
        <div>SSID:</div>
        <div><input maxlength="60"  name="0" size="45" type="text"></div>
      </div>
      <div class="tr">
        <div>Password:</div>
        <div><input maxlength="60"  name="1" size="45" type="text"></div>
      </div>
      <div class="tr header">
        <div>MQTT Server</div>
        <div><hr/></div>
      </div>
      <div class="tr">
        <div>Host:</div>
        <div><input maxlength="60"  name="2" size="45" type="text"></div>
      </div>
      <div class="tr">
        <div>Port:</div>
        <div><input maxlength="60"  name="3" size="10" type="text"></div>
      </div>
      <div class="tr">
        <div>User:</div>
        <div><input maxlength="60"  name="4" size="45" type="text"></div>
      </div>
      <div class="tr">
        <div>Password:</div>
        <div><input maxlength="60"  name="5" size="45" type="text"></div>
      </div>
    
      <div class="tr header">
        <div>MQTT Client</div>
        <div><hr/></div>
      </div>
      <div class="tr">
        <div>Name:</div>
        <div><input maxlength="60"  name="6" size="45" type="text"></div>
      </div>
      <div class="tr">
        <div>Topic:</div>
        <div><input maxlength="60"  name="7" size="45" type="text"></div>
      </div>
      <div class="tr">
        <div>Interval in minutes:</div>
        <div><input maxlength="60"  name="8" size="45" type="text"></div>
      </div>
      <div class="tr">
        <div>&nbsp;</div>
        <div>&nbsp;</div>
      </div>
      <div class="tr footer">
        <div>&nbsp;</div>
        <div><span id="successMsg"></span> <input type="submit" value="Save" ></div>
      </div>
    </div>
    </form> 
  </div>

  <script type="text/javascript">
  for (i=0;i<9;i++) document.getElementsByTagName("input")[i].value=data[i];
  document.getElementById("successMsg").innerHTML=successMsg;
  </script>
</body></html>
)=====" ;
