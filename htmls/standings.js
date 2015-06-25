$.getJSON("./jsons/full_res.json").done( function( standings ) {
    table = $(".container");
    for (i = 0; i < standings.length; i++) {
        str = "<div class=\"table-row\">";
        str += "<div class=\"col\">" + (i + 1).toString() + "</div>";
        str += "<div class=\"col\"> <a href=\"./player_results/player_res.html?player_id=" + standings[i].player_id + "\">" +  
        standings[i].name + "</a> </div>";
        str += "<div class=\"col\">" + standings[i].rating.toString() + "</div>";
        str += "</div>";
        table.append(str);
        if (i % 2 == 1) {
            table.children().last().css("background-color", "#FFDDEE");
        }
    }
});
