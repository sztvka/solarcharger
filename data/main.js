var a = [2, 3, 4, 5];
a.forEach(element => {
    console.log(element*3);    
});
var cnt = 0;
const ctx = document.getElementById('myChart');

var voltage = new Chart(ctx, {
  type: 'line',
  data: {
    labels: [1],
    datasets: [{
      label: 'Value',
      data: [0],
      borderWidth: 1
    }]
  },
  options: {
    scales: {
      y: {
        beginAtZero: true
      }
    }
  }
});


updateGauges = (obj) =>{
    cnt++;
    if(voltage.data.labels.length>19){
        voltage.data.labels = [];
        voltage.data.datasets[0].data = [];
        cnt = 1;
    };
    
    voltage.data.labels.push(cnt);
    voltage.data.datasets[0].data.push(obj.ina1.voltage);
    voltage.update();
};





fetchData = () => {
    fetch('data', {
        method: 'GET',
        headers: {
            'Accept': 'application/json',
        },
    })
    .then(response => response.json())
    .then(text => {
        updateGauges(text);

    })
};

window.onload = function() {
    document.getElementsByClassName("mask")[0].style = "visibility: hidden";;
  //  setInterval(fetchData, 1000);
};


function getNewData(baseval, yrange) {
  var newTime = baseval+3600000;
  return {
    x: newTime,
    y: Math.floor(Math.random() * (yrange.max - yrange.min + 1)) + yrange.min
  };
}

window.Apex = {
  tooltip: {
    theme: "dark",
    x: {
      formatter: function (val) {
        var d = new Date(val);
        var min = () => {
          if (d.getUTCMinutes()<10){
            return "0"+d.getUTCMinutes();
          }
          else return d.getUTCMinutes();
        }
        var parsed = d.getUTCHours()+":"+min()+":"+d.getUTCSeconds();
        return parsed
      }
    }
  }
}


var data = [];
var options = {
  series: [{
    name: "Power",
    data: data.slice()
}],
  chart: {
    id: 'realtime',
    height: 350,
    type: 'line',
    animations: {
      enabled: true,
      easing: 'linear',
      dynamicAnimation: {
        speed: 1000
      }
    },
    toolbar: {
      show: false
    },
    zoom: {
      enabled: false
    }
  },
  dataLabels: {
    enabled: false
  },
  stroke: {
    curve: 'smooth'
  },
  title: {
    text: 'Dynamic Updating Chart',
    align: 'left'
  },
  markers: {
    size: 0
  },
  xaxis: {
    type: 'datetime',
    range: 5000,
  },
  yaxis: {
    max: 40
  },
  legend: {
    show: false
  },
};

var chart = new ApexCharts(document.querySelector("#chart"), options);
chart.render();


window.setInterval(function () {
  var b = {};
  b = getNewData(Date.now(), {min: 0, max:30});
  data.push([b.x,b.y]);

chart.updateSeries([{
  data: data
}])
}, 1000)

