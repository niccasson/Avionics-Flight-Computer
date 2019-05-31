document.getElementById("downloadButton").addEventListener("click",downloadData);



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
        success: function () {}
     });

}