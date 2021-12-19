const palette = ["#000000","#1D2B53","#7E2553","#008751",
               "#AB5236","#5F574F","#C2C3C7","#FFF1E8",
               "#FF004D","#FFA300","#FFEC27","#00E436",
               "#29ADFF","#83769C","#FF77A8","#FFCCAA"]

//Sprite canvas pixel size 16x16
const editor_width_px=16;
const editor_height_px=16;

function init() {
  document.getElementById("delete_button").addEventListener("click", httpGetDelete);
  listFiles();
}

function listFiles() {
  var xmlHttp = new XMLHttpRequest();
  xmlHttp.open( "GET", "getList.html", false ); // false for synchronous request
  xmlHttp.send( null );

  let response = xmlHttp.responseText;
  document.getElementById("loader").style.display = "none";

  if(response.includes('|')) {
    const sketchList = response.split("|");

    for (let sketchIndex in sketchList) {
      if(sketchList[sketchIndex].includes(',')) {
        const sketchData = sketchList[sketchIndex].split(",");
          generateThumbnailHtml(sketchData[0],sketchData[1]);
      }
    }
  }
}

function generateThumbnailHtml(fileName, sketchString) {
  let displayName = fileName.replace("/imgs/", "").replace(".txt", "");
  let contentDiv = document.getElementById('content');
  let thumbnailDiv = document.createElement('div');
  thumbnailDiv.className = "thumbnail";
  let check = document.createElement("input");
  check.type = "checkBox";
  check.id = displayName;
  let canvas = document.createElement('canvas');
  canvas.id = "canvas_" + displayName;
  canvas.width = 64;
  canvas.height = 64;
  canvas.style.border = "1px solid";

  thumbnailDiv.appendChild(check);
  thumbnailDiv.appendChild(document.createTextNode(displayName));
  thumbnailDiv.appendChild(document.createElement('br'));
  thumbnailDiv.appendChild(canvas);
  contentDiv.appendChild(thumbnailDiv);

  renderThumbail(sketchString, canvas.id);
}

function renderThumbail(sketchString, canvasId) {
  let canvas = document.getElementById(canvasId);
  let thumbContext = canvas.getContext("2d");
  let spriteWidth = thumbContext.canvas.clientWidth;
  let spriteHeight = thumbContext.canvas.clientHeight;
  let square_width = spriteWidth/editor_width_px;
  let square_height = spriteHeight/editor_height_px;
  let pixelIndex = 0;

  for(row_count = editor_height_px - 1 ; row_count >= 0 ; row_count--) {
    if(row_count % 2 == 0) {
      for(column_count = editor_width_px - 1 ; column_count >= 0 ; column_count--) {
        let square_x = square_width*column_count;
        let square_y = square_height*row_count;
        thumbContext.fillStyle = palette[sketchString.charCodeAt(pixelIndex) - 65];
        thumbContext.fillRect(square_x, square_y, square_width, square_height);
        pixelIndex++;
      }
    } else {
      for(column_count = 0 ; column_count < editor_width_px ; column_count++) {
        let square_x = square_width*column_count;
        let square_y = square_height*row_count;
        thumbContext.fillStyle = palette[sketchString.charCodeAt(pixelIndex) - 65];
        thumbContext.fillRect(square_x, square_y, square_width, square_height);
        pixelIndex++;
      }
    }
  }
}

function httpGetDelete(theUrl) {
  //Get the list of all the checked checkBoxes
  let allCheckBoxes = document.getElementsByTagName("input");
  let files = "";
  for(checkBoxIndex in allCheckBoxes) {
    let checkBox = allCheckBoxes[checkBoxIndex];
    if(checkBox.type == "checkbox" && checkBox.checked) {
      files += checkBox.id + ",";
    }
  }

  //Send the query
  var xmlHttp = new XMLHttpRequest();
  xmlHttp.open( "GET", "delete.html?sketchlist=" + files, false ); // false for synchronous request
  xmlHttp.send( null );
  return xmlHttp.responseText;
}
