//Color palette inspired by PICO-8
//var color_palette={0xAABBCC,0xBBCCDD};
const palette = ["#000000","#1D2B53","#7E2553","#008751",
               "#AB5236","#5F574F","#C2C3C7","#FFF1E8",
               "#FF004D","#FFA300","#FFEC27","#00E436",
               "#29ADFF","#83769C","#FF77A8","#FFCCAA"]

//Sprite canvas pixel size 16x16
const editor_width_px=16;
const editor_height_px=16;

const log = true;

var drawingMatrix = new Array(editor_width_px);
for (var i = 0; i < editor_width_px; i++) {
  drawingMatrix[i] = new Array(editor_height_px);
}

let spriteCanvas;
let spriteContext;

let paletteCanvas;
let paletteContext;

var selectedColor = "#000000";
var selectedPaletteIndex = 0;

function init() {
  spriteCanvas = document.getElementById("sprite");
  spriteCanvas.addEventListener("mousedown", function(e){
    spriteMouseDown(e);
  });
  spriteCanvas.addEventListener("mousemove", function(e){
    spriteMouseMove(e);
  });
  spriteCanvas.addEventListener("mouseup", function(e){
    spriteMouseUp(e);
  });

  spriteCanvas.addEventListener("mouseout", function(e){
    spriteMouseOut(e);
  });

  spriteContext = spriteCanvas.getContext("2d");

  paletteCanvas = document.getElementById("palette");
  paletteCanvas.addEventListener("mousedown", function(e){
    paletteClick(e);
  });
  paletteContext = paletteCanvas.getContext("2d");

  initDrawingMatrix(selectedPaletteIndex);
  drawSpriteGrid(selectedColor);
  drawColorPalette();

  document.getElementById("fill_button").addEventListener("click", fill);
  document.getElementById("save_button").addEventListener("click", httpGetSave);
  document.getElementById("display_button").addEventListener("click", httpGetDisplay);
}

function initDrawingMatrix(paletteIndex) {
  for (var x = 0; x < editor_width_px; x++) {
    for (var y = 0; y < editor_height_px; y++) {
      drawingMatrix[x][y] = paletteIndex;
    }
  }
}

//Draw black squares and white borders
//to show an empty pixel grid
function drawSpriteGrid(color) {
  let spriteWidth = spriteContext.canvas.clientWidth;
  let spriteHeight = spriteContext.canvas.clientHeight;
  let square_width = spriteWidth/editor_width_px;
  let square_height = spriteHeight/editor_height_px;

  for(row_count = 0 ; row_count < editor_height_px ; row_count++) {
    for(column_count = 0 ; column_count < editor_width_px ; column_count++) {
      let square_x = square_width*column_count;
      let square_y = square_height*row_count;
      spriteContext.fillStyle = color;
      if(color == "#000000")
        spriteContext.strokeStyle = "White";
      else
        spriteContext.strokeStyle = "Black";
      spriteContext.fillRect(square_x, square_y, square_width, square_height);
      spriteContext.strokeRect(square_x, square_y, square_width, square_height);
    }
  }
}

function drawColorPalette() {
  let paletteWidth = paletteContext.canvas.clientWidth;
  let paletteHeight = paletteContext.canvas.clientHeight;
  let square_width = paletteWidth/4;
  let square_height = paletteHeight/4;
  let paletteIndex = 0;

  for(row_count = 0 ; row_count < 4 ; row_count++) {
    for(column_count = 0 ; column_count < 4 ; column_count++) {
      let square_x = square_width*column_count;
      let square_y = square_height*row_count;
      paletteContext.fillStyle = palette[paletteIndex];
      paletteContext.fillRect(square_x, square_y, square_width, square_height);
      paletteIndex++;
    }
  }
}

var isDrawing = false;

function spriteMouseDown(event) {
  if(log) console.log("Sprite click !");
  isDrawing = true;
  draw();
}

function spriteMouseMove(event) {
  //Curseur coloré
  let rect = spriteCanvas.getBoundingClientRect();
  let x = event.clientX - rect.left;
  let y = event.clientY - rect.top;
  spriteContext.fillStyle = selectedColor;
  if(selectedColor == "#000000") {
    spriteContext.strokeStyle = "White";
  } else {
    spriteContext.strokeStyle = "Black";
  }
  paletteContext.beginPath();
  paletteContext.arc(50, 300, 15, 0, 2 * Math.PI, false); // full circle
  paletteContext.fill();
  paletteContext.stroke();

  if(isDrawing) draw();
}

function spriteMouseUp(event) {
  isDrawing = false;
}

function spriteMouseOut(event) {
  isDrawing = false;
}

function draw(){
  let rect = spriteCanvas.getBoundingClientRect();
  let x = event.clientX - rect.left;
  let y = event.clientY - rect.top;

  let spriteWidth = spriteContext.canvas.clientWidth;
  let spriteHeight = spriteContext.canvas.clientHeight;
  let square_width = spriteWidth/editor_width_px;
  let square_height = spriteHeight/editor_height_px;

  for(row_count = 0 ; row_count < editor_height_px ; row_count++) {
    for(column_count = 0 ; column_count < editor_width_px ; column_count++) {
      let square_x = square_width*column_count;
      let square_y = square_height*row_count;

      spriteContext.fillStyle = selectedColor;
      if(selectedColor == "#000000") {
        spriteContext.strokeStyle = "White";
      } else {
        spriteContext.strokeStyle = "Black";
      }

      if(x > square_x
        && x < square_x + square_width
        && y > square_y
        && y < square_y + square_height) {
          if(log) console.log("    Column : " + column_count + " , Row : " + row_count);
          drawingMatrix[column_count][row_count] = selectedPaletteIndex;
          spriteContext.clearRect(square_x, square_y, square_width, square_height);
          spriteContext.fillRect(square_x, square_y, square_width, square_height);
          spriteContext.strokeRect(square_x, square_y, square_width, square_height);
      }
    }
  }
}

function paletteClick(event) {
  if(log) console.log("Palette click !");
  selectedColor = "#000000";
  selectedPaletteIndex = 0;
  let rect = paletteCanvas.getBoundingClientRect();
  let x = event.clientX - rect.left;
  let y = event.clientY - rect.top;

  var imgData = paletteContext.getImageData(x,y,1,1).data;
  var hexCode = "#" + ("000000" + rgbToHex(imgData[0], imgData[1], imgData[2])).slice(-6);
  hexCode = hexCode.toUpperCase();
  if(log) console.log("    color: " + hexCode);

  for(index=0 ; index < palette.length ; index++) {
    if(palette[index] == hexCode) {
      selectedColor = palette[index];
      selectedPaletteIndex = index;
      if(log) console.log("    palette index: " + selectedPaletteIndex);
    }
  }

  document.getElementById("fill_button").style.backgroundColor = selectedColor;
}

function fill() {
  if(log) console.log("Fill click !");
  initDrawingMatrix(selectedPaletteIndex);
  drawSpriteGrid(selectedColor);
  if(log) printDrawingMatrix();
}

// function httpGetAsync(theUrl, callback)
// {
//     var xmlHttp = new XMLHttpRequest();
//     xmlHttp.onreadystatechange = function() {
//         if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
//             callback(xmlHttp.responseText);
//     }
//     xmlHttp.open("GET", "./save.html", true); // true for asynchronous
//     xmlHttp.send(null);
// }

function httpGetDisplay(theUrl)
{
  var xmlHttp = new XMLHttpRequest();
  xmlHttp.open( "GET", "display.html?sketch=" + printDrawingMatrixForESP(), false ); // false for synchronous request
  xmlHttp.send( null );
  return xmlHttp.responseText;
}

function httpGetSave(theUrl)
{
  var fileName = prompt("Quel nom veut-tu donner à ton dessin ?", "");
  fileName = fileName.trim().substring(0,50).replace(/[\W_]+/g,"");

  var xmlHttp = new XMLHttpRequest();
  xmlHttp.open( "GET", "save.html?sketch=" + printDrawingMatrixForESP() + "&fileName=" + fileName, false ); // false for synchronous request
  xmlHttp.send( null );
  return xmlHttp.responseText;
}

function httpGetList(theUrl)
{
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open( "GET", "list.html", false ); // false for synchronous request
    xmlHttp.send( null );
    return xmlHttp.responseText;
}

//*************************************************************************
//                          TOOLS
//*************************************************************************

function rgbToHex(r, g, b) {
  if (r > 255 || g > 255 || b > 255)
    throw "Invalid color component";
  return ((r << 16) | (g << 8) | b).toString(16);
}

//ESP displays from bottom to top row.
//Row 15 from left to right
//Row 14 from right to left
//etc...
function printDrawingMatrixForESP() {
  let text = "";
  for (var y = editor_height_px - 1; y >= 0; y--) {
    //for even rows go right to left
    if(y % 2  == 0) {
      for (var x = editor_width_px - 1; x >= 0; x--) {
        text += String.fromCharCode(drawingMatrix[x][y] + 65);
      }

    } else {
      //for odd rows, go left to right
      for (var x = 0; x < editor_width_px; x++) {
        text += String.fromCharCode(drawingMatrix[x][y] + 65);
      }
    }
  }
  if(log) console.log(text);
  return text;
}
