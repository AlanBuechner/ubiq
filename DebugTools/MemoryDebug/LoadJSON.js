$(document).ready(function()
{
    $.getJSON('test.json', function(data)
    {
        console.log(data);
    });
});