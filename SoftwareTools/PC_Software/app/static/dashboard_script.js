
document.querySelector(".terminal_entry").addEventListener("keyup",terminalSubmit);
document.getElementById("refreshCOM").addEventListener("click",getComPorts);


function terminalSubmit(e){
console.log("key press");
    if(e.keyCode === 13){


    var xhttp = new XMLHttpRequest();

     var entry_txt = document.getElementById("term_entry");
     var dsp_txt = document.getElementById("term_dsp");

    // post('/',{text: entry_txt.value});

          $.ajax({
            type: 'post',
            url: '/terminal',
            data: entry_txt.value,
            success: function () {
              alert('form was submitted');
            }
          });

        // xhttp.open("POST",'/terminal',true);
        // xhttp.send("text="+entry_txt.value);

         console.log("Enter pressed");
        // console.log(entry_txt.value);
        // console.log(dsp_txt.value);
        dsp_txt.value += entry_txt.value;
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
            url: '/connect',

            success: function (data) {

                console.log(data);

                //If there are COM ports, add radio buttons to be able select one.
                if(data.length>0) {

                    //Get the radio button form from the html page.
                    //var radioFormName = document.getElementById("comPortList");

                    var comList = document.getElementById("comPortList");
                    comList.options[0].remove();

                    var i;
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
                }
            }
          });
}