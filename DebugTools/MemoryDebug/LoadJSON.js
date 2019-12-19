$(document).ready(function()
{
    var drag = false;

    var wheelPos = 0;
    var scale = Math.pow(2, wheelPos);
    var index = 0;
    var xoffset = 0.0;
    var lastX = 0;

    var Allocator;

    window.addEventListener('resize', function(){
        resizeCanvas();
        displayData(Allocator);
    });

    $(document).bind("mousewheel",function(e){
        var lastScale = scale;
        var targetScale = Math.pow(2, wheelPos + e.originalEvent.wheelDelta / 120);
        if(targetScale <= 0.0)
            return;
        scale = targetScale;
        wheelPos += e.originalEvent.wheelDelta / 120;
        recalculateOffset();
        displayData(Allocator);
    });

    var initMouseX;

    $(document).bind("mousedown", function(e){
        if(e.button == 0){
            drag = true;
            initMouseX = e.pageX;
        }
    });

    $(document).bind("mouseup", function(e){
        if(e.button == 0){
            drag = false;
            displayData(Allocator);
        }
    });

    $(document).bind("mousemove", function(e){
        if(drag && e.altKey)
        {
            index -= (lastX - e.pageX)/scale;
            var windowWidth = parseInt($("body").css('width').replace("px", ""));
            if(xoffset > windowWidth){
                xoffset = windowWidth;
            }
            recalculateOffset();
            displayData(Allocator);
        }
        else if(drag)
        {
            var data = document.getElementById("data");
            var ctx = data.getContext("2d");
            displayData(Allocator);
            var ypos = 55;
            var height = 10;
            ctx.fillStyle = "#4287f5aa";
            ctx.fillRect(initMouseX-8, 0, e.pageX-initMouseX, data.height);
            ctx.fillStyle = "#000000";
            ctx.fillRect(initMouseX-8, ypos, e.pageX-initMouseX, 2);
            ctx.fillRect(initMouseX-8, ypos-(height/2), 2, height+2);
            ctx.fillRect(e.pageX-10, ypos-(height/2), 2, height+2);
            ctx.font = "10px Arial";
            var text = ((e.pageX-initMouseX)/scale) + "bytes";
            ctx.fillText(text, initMouseX + ((e.pageX-initMouseX)/2) - ((text.length*5)/2), ypos + 20);
        }
        lastX = e.pageX;
    });

    $("#btnLoad").click(function(){
        var input, file, fr;

        if (typeof window.FileReader !== 'function') {
            alert("The file API isn't supported on this browser yet.");
            return;
        }

        input = document.getElementById('fileinput');
        if (!input) {
            alert("Um, couldn't find the fileinput element.");
        }
        else if (!input.files) {
            alert("This browser doesn't seem to support the `files` property of file inputs.");
        }
        else if (!input.files[0]) {
            alert("Please select a file before clicking 'Load'");
        }
        else {
            file = input.files[0];
            fr = new FileReader();
            fr.onload = receivedText;
            fr.readAsText(file);
        }

        function receivedText(e) {
            let lines = e.target.result;
            var newArr = JSON.parse(lines);

            wheelPos = 0;
            scale = Math.pow(2, wheelPos);
            index = (-parseInt($("body").css('width').replace("px", ""))/2)/scale;
            lastX = 0;

            recalculateOffset();

            displayData(newArr);
        }
    });

    function recalculateOffset()
    {
        var windowWidth = parseInt($("body").css('width').replace("px", ""));
        xoffset = index * scale;
        if(xoffset > windowWidth){
            xoffset = windowWidth;
        }
    }

    function displayData(allocator)
    {
        Allocator = allocator
        if(allocator.type == "FreeList")
        {
            displayFreeList(allocator.allocator);
        }
        else
        {
            alert("allocator type is not saproted");
        }
    }

    function displayFreeList(a)
    {
        var data = document.getElementById("data");
        var ctx = data.getContext("2d");
        var windowWidth = parseInt($("body").css('width').replace("px", ""));
        var windowMid = windowWidth/2;

        ctx.clearRect(0, 0, data.width, data.height);

        var allocatorHeight = 30;
        const deltaYOffset = 50;
        var newYOffset = deltaYOffset;

        a.forEach(function(e) {
            AddAllocator(e);
        });

        function AddAllocator(e)
        {
            ctx.fillStyle = "#000000";
            ctx.font = "30px Arial";
            ctx.fillText(e.name, 50, newYOffset - 10);

            ctx.fillRect(0, newYOffset, windowWidth, 2);

            var segments = (windowWidth/scale);
            var segmentSpasing = scale;

            while(segments > 100)
            {
                segments /= 10;
                segmentSpasing *= 10;
            }
            
            for(var i = 0; i < segments; i++)
            {
                ctx.fillRect(i*segmentSpasing + (xoffset%segmentSpasing) + windowMid, newYOffset, 2, 10);
                ctx.fillRect(windowMid - (i*segmentSpasing - (xoffset%segmentSpasing)), newYOffset, 2, 10);
            }

            newYOffset += deltaYOffset;

            var headerSize = e.headerSize;

            e.SnapShots.forEach(function(snapShot){
                ctx.fillStyle = "#FF0000";
                ctx.fillRect(xoffset + windowMid, newYOffset, e.size * scale, allocatorHeight);

                snapShot.alloc.forEach(function(alloc){
                    AddAllocation(alloc);
                });
                newYOffset += allocatorHeight + deltaYOffset/2;
            });

            function AddAllocation(alloc)
            {
                var start = (alloc.start + headerSize);
                var headerStart = (alloc.start * scale) + xoffset + windowMid;
                var xStart = (start * scale) + xoffset + windowMid;
                var size = (alloc.header[0].size - headerSize - 1) * scale;
                var footerStart = ((alloc.start + alloc.header[0].size - 1) * scale) + xoffset + windowMid;

                // draw header
                ctx.fillStyle = "#d10099";
                ctx.fillRect(headerStart, newYOffset, headerSize * scale, allocatorHeight);

                drawTextinRegen("header", headerStart, newYOffset, headerSize * scale, allocatorHeight);
                
                // draw body
                ctx.fillStyle = "#00FF00";
                ctx.fillRect(xStart, newYOffset, size, allocatorHeight);

                drawTextinRegen(alloc.body, xStart, newYOffset, size, allocatorHeight);

                // draw footer
                ctx.fillStyle = "#1D32FF";
                ctx.fillRect(footerStart, newYOffset, scale, allocatorHeight);

                drawTextinRegen(alloc.next.toString(), footerStart, newYOffset, scale, allocatorHeight);

                // draw arrow to next
                ctx.fillStyle = "#000000";
                ctx.fillRect(footerStart + (scale/2), newYOffset+allocatorHeight, 2, allocatorHeight/2);

                if(alloc.next != 0)
                {
                    ctx.fillRect((alloc.next * scale) + xoffset + windowMid, newYOffset+allocatorHeight, 2, allocatorHeight/2);
                    ctx.fillRect(footerStart + (scale/2), newYOffset+(allocatorHeight*(3/2)), ((alloc.next * scale) + xoffset + windowMid) - (footerStart + (scale/2)) + 2, 2);
                }

                function drawTextinRegen(text, startx, starty, sizex, sizey)
                {
                    var newText = "";
                    for(var i = 0; i < text.length; i++)
                    {
                        if((i+1) * 10 > sizex)
                        {
                            break;
                        }
                        newText += text.charAt(i);
                    }

                    ctx.font = "10px Arial";
                    ctx.fillStyle = "#000000";
                    ctx.fillText(newText, startx + (sizex/2) - ((newText.length*5)/2), starty+(sizey/2));
                }
            }

            newYOffset += allocatorHeight + deltaYOffset;
        }
    }

    function setCanvasScalingFactor() {
        return window.devicePixelRatio || 1;
     }
     
     function resizeCanvas() {

        var aWrapper = document.getElementById("wrapper");
        var canvas = document.getElementById("data");

         //Gets the devicePixelRatio
         var pixelRatio = setCanvasScalingFactor();
     
         if (window.innerHeight < window.innerWidth) {
             //Makes the canvas 100% of the viewport width
             var width = Math.round(1.0 * window.innerWidth) - 2 * parseInt($("body").css('margin').replace("px", ""));
         }
         else {
             //Makes the canvas 100% of the viewport height
             var width = Math.round(1.0 * window.innerHeight);
         }
     
         //This is done in order to maintain the 1:1 aspect ratio, adjust as needed
         var height = width * (window.innerHeight / window.innerWidth) - $("#jsonFile").height() - 2 * parseInt($("body").css('margin').replace("px", ""));
     
         //This will be used to downscale the canvas element when devicePixelRatio > 1
         aWrapper.style.width = width + "px";
         aWrapper.style.height = height + "px";
     
         canvas.width = width * pixelRatio;
         canvas.height = height * pixelRatio;
     }
     
     resizeCanvas();

});