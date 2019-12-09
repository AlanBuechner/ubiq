$(document).ready(function()
{
    var drag = false;
    var alt = true;

    var scale = 100.0;
    var xoffset = 0.0;
    var wheelPos = 0;
    var lastX = 0;

    var Allocator;

    window.addEventListener('resize', function(){
        resizeCanvas();
        displayData(Allocator);
    });

    $("#data").bind("mousewheel",function(e){
        var targetScale = Math.pow(2, wheelPos + e.originalEvent.wheelDelta / 120);
        if(targetScale <= 0.0)
            return;
        scale = targetScale;
        wheelPos += e.originalEvent.wheelDelta / 120;
        xoffset -= lastX - scale;
        if(xoffset > 0)
            xoffset = 0;
        displayData(Allocator);
    });

    $("#data").bind("mousedown", function(e){
        if(e.button == 0){
            drag = true;
        }
    });

    $("#data").bind("mouseup", function(e){
        if(e.button == 0){
            drag = false;
        }
    });

    $("#data").bind("mousemove", function(e){
        if(drag && alt)
        {
            xoffset -= (lastX - e.pageX);
            if(xoffset > 0)
                xoffset = 0;
            displayData(Allocator);
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
            displayData(newArr);
        }
    });

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

            e.SnapShots.forEach(function(snapShot){
                ctx.fillStyle = "#FF0000";
                ctx.fillRect(xoffset, newYOffset, e.size * scale, allocatorHeight);

                snapShot.alloc.forEach(function(alloc){
                    AddAllocation(alloc);
                });
                newYOffset += allocatorHeight + deltaYOffset/2;
            });

            function AddAllocation(alloc)
            {
                ctx.fillStyle = "#00FF00";
                ctx.fillRect((alloc.start * scale) + xoffset, newYOffset, (alloc.end - alloc.start) * scale, allocatorHeight);
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