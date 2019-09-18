document.getElementById("downloadButton").addEventListener("click",downloadData);
document.getElementById("stopDownloadButton").addEventListener("click",stopDownloadData);



var ul = document.getElementById("navbar");
var items = ul.getElementsByTagName("li");
for (var i = 0; i < items.length; ++i) {

    console.log("name: "+items[i].textContent+" class: "+items[i].getElementsByTagName("a").className);
  if(items[i].getElementsByTagName("a").className == "active"){
      console.log("current active: "+items[i].textContent)
      items[i].getElementsByTagName("a").className ="";
  }
  if(items[i].textContent == "Download Data"){
      items[i].getElementsByTagName("a").className = "active";
  }
}

function downloadData(e){

    var date = new Date();

    var csv_name = document.getElementById("csvFileNameEntry").value;
    csv_name += "_"+date.getFullYear()+"_"+(date.getMonth()+1)+"_"+date.getDate()+"_"+date.getHours()+"h"+date.getMinutes()+"m.csv";

    var log_name = document.getElementById("logFileNameEntry").value;
    log_name += ".log"

    $.ajax({
        type: 'post',
        url: '/DownloadData',
        data: JSON.stringify({"csvName": csv_name,"logName":log_name}),
        success: function () {

            alert("Download will start soon. LED D3 will turn on when the transfer has started. Press the Stop Download button once the LED turns off. ");
        }
     });

}

function stopDownloadData(e){

        $.ajax({
        type: 'get',
        url: '/StopDownloadData',
        success: function () {
        alert("Download Complete!");
        }
     });

}