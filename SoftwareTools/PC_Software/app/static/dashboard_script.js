
document.querySelector(".terminal_entry").addEventListener("keyup",terminalSubmit);
document.getElementById("refreshCOM").addEventListener("click",getComPorts);
document.getElementById("ConnectSerialButton").addEventListener("click",connectFunc);
document.getElementById("refreshTerminalBtn").addEventListener("click",refreshTerminal);


var comPortGlobal;

getComPorts();


function on_refresh(){
 $.ajax({
        type: 'get',
        url: '/readGlobal',
        success: function (data) {
            data = JSON.parse(data);
            var serialState = data["serialOpen"];

            var connectButton =document.getElementById("ConnectSerialButton");
        if(serialState == true){
                connectButton.value = "Disconnect";
            }
        else{
                connectButton.value = "Connect";
        }

        var baudrate = data["baudRate"];
        var baudRateBox = document.getElementById("baudEntry");
        baudRateBox.value = baudrate;

        var comPort = data["comPort"];
        var comBox = document.getElementById("comPortList");

        for (var i = 0;i < comBox.options.length;  i++)
            {
                console.log(comBox.options[i].value+" | "+comPort);
                if (comBox.options[i].value == comPort) {
                    comBox.options[i].selected = true;
                    console.log("comport match");
                }
            }
        window.comPortGlobal = comPort;

        console.log("Refresh: serial state -> "+serialState+ " baudrate -> "+baudrate+ " comPort -> "+comPort);
    }
 });
}
function refreshTerminal(){

    var dsp = document.getElementById("term_dsp");
    dsp.value = "";
}

function refresh() {


    var dsp_txt = document.getElementById("term_dsp");
              $.ajax({
            type: 'get',
            url: '/terminal_in',
            success: function (data) {
                console.log("refresh")
                console.log(data)

              dsp_txt.value += data;
              var textarea = document.getElementById('term_dsp');
              textarea.scrollTop = textarea.scrollHeight;

            }
          });

    setTimeout(refresh, 500);
    // ...
}

// initial call, or just call refresh directly
setTimeout(refresh, 1000);


function connectFunc(e){

    var comPortList = document.getElementById("comPortList");
    var comPort =comPortList.options[comPortList.selectedIndex].value;

    var baudRateBox = document.getElementById("baudEntry");
    var baudRate = baudRateBox.value;

    console.log(comPort);
    console.log(baudRate);

     $.ajax({
            type: 'get',
            url: '/readGlobal',
            success: function (data) {
                data = JSON.parse(data);
                var serialState = data["serialOpen"];
                console.log(typeof(data));
                console.log(data);
                console.log(data.toString());

                   if(serialState == false) {
                        // serialState = true;
                        $.ajax({
                            type: 'post',
                            url: '/connect',
                            data: JSON.stringify({"baudrate": baudRate, "COM": comPort}),
                            success: function (data) {


                                if (data['res'] == 0) {
                                    alert("Failed!");
                                }
                                if (data['res'] == 1) {
                                    var button = document.getElementById("ConnectSerialButton");
                                    button.value = "Disconnect";
                                    window.serial_state_global = true;
                                    console.log("Connected");


                                    $.ajax({
                                            type: 'post',
                                            url: '/writeGlobal',
                                            data: JSON.stringify({"serialOpen":true,"comPort": comPort,"baudRate":baudRate}),
                                            success: function () {
                                            }
                                    });
                                }

                            }
                        })
                    }
                else if (serialState == true){

                    $.ajax({
                        type: 'get',
                        url: '/disconnect',
                        success: function (data) {

                         alert("Disconnect");
                        }
                    });

                    var button = document.getElementById("ConnectSerialButton");
                    button.value = "Connect";
                    window.serial_state_global = false;
                    // serialState = false;
                    $.ajax({
                        type: 'post',
                        url: '/writeGlobal',
                        data: JSON.stringify({"serialOpen":false,"comPort": "","baudRate":""}),
                        success: function () {}
                     });
                }



            }
        });


    console.log(window.serial_state_global);
}



function terminalSubmit(e){
console.log("key press");
    if(e.keyCode === 13){


    var xhttp = new XMLHttpRequest();

     var entry_txt = document.getElementById("term_entry");
     var dsp_txt = document.getElementById("term_dsp");

    // post('/',{text: entry_txt.value});
        //SENDING COMMAND TO FLIGHT COMPUTER REQUIRES A \r !!!!!!!
        var command = (entry_txt.value).replace("\n","") +"\r";
          $.ajax({
            type: 'post',
            url: '/terminal_out',
            data: JSON.stringify ({'text':command}),
            success: function (data) {

              //dsp_txt.value += data;

            }
          });

        // xhttp.open("POST",'/terminal',true);
        // xhttp.send("text="+entry_txt.value);

         console.log("Enter pressed");
        // console.log(entry_txt.value);
        // console.log(dsp_txt.value);

        // console.log(dsp_txt.value);
        entry_txt.value = "";

    }
}

function post(path, params, method='post') {

  // The rest of this code assumes you are not using a library.
  // It can be made less wordy if you use one.
  const form = document.createElement('form');
  form.method = method;
  form.action = path;

  for (const key in params) {
    if (params.hasOwnProperty(key)) {
      const hiddenField = document.createElement('input');
      hiddenField.type = 'hidden';
      hiddenField.name = key;
      hiddenField.value = params[key];

      form.appendChild(hiddenField);
    }
  }

  document.body.appendChild(form);
  form.submit();
}

function getComPorts(){

    $.ajax({
            type: 'get',
            url: '/comports',

            success: function (data) {

                console.log(data);

                //If there are COM ports, add radio buttons to be able select one.
                if(data.length>0) {

                    //Get the radio button form from the html page.
                    //var radioFormName = document.getElementById("comPortList");

                    var comList = document.getElementById("comPortList");


                    var i;
                    for (option in comList.options){
                        comList.options.remove(0);
                    }

                    for (i = 0; i < data.length; i++) {

                        //create the button
                        // var radio = document.createElement("input");
                        // radio.type = "radio";
                        // radio.name = "comPorts";
                        // radio.class = "radioButtons";
                        // radio.value = i;
                        // radio.id = "choice" + i;
                        //
                        // //create a div to hold the button and button text.
                        // var radioText = document.createElement("div");
                        // radioText.id = "c" + i;
                        // radioText.class = "choiceText";
                        // radioText.innerHTML = data[i];
                        //
                        // radioText.appendChild(radio);
                        var option = document.createElement("option");
                        option.value=data[i];
                        option.text=data[i];
                        comList.appendChild(option);
                    }
                    //radioFormName.appendChild(comList);
                     on_refresh();
                }
            }
          });


}