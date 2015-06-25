var plot;


player_id = $.url("?player_id");
$.getJSON("../jsons/player_" + player_id.toString() + "_res.json").done(function(data) {
    console.log(data);
    $("h1").text(data.name);
    $("#rating").text(data.rating.toString());
    $("#place").text(data.place.toString());
    $("#won").text(data.won.toString() + "/" + data.games.length.toString());
    ratings = [];
    games = data.games;
    for (i = 0; i < games.length; i++) {
        for (j = 0; j < games[i].players.length; j++) {
            if (games[i].players[j].player_id == player_id) {
                ratings.push([i + 1, games[i].players[j].new_rating]);
            }
        }
    }
    plot = $.jqplot("chartdiv", [ratings], {
        axes: {
            xaxis: {
                padMin: 1 ,
                min: 0,            
                tickOptions: {
                    formatString: '%d',
                }
            }
        },
        highlighter: {
            show: true,
            sizeAdjust: 7.5,
        },
        cursor: {
            show: true,
            zoom: true,
        },
        seriesDefaults: {    
            markerOptions: {
                linewidth: 2,
                size: 7,
            }
        }
    });
    $('#chartdiv').bind('jqplotDataClick', function (ev, seriesIndex, pointIndex, data) {                
        window.location.hash = pointIndex + 1;
        $('html, body').animate({
            scrollTop: $("#tbl").children().eq(pointIndex + 1).offset().top,
        }, 500);
    });
    $('.button-reset').click(function() { plot.resetZoom() });
    
    
    for (i = 0; i < games.length; i++) {        
        str = "<div class=\"table-row\">";
        
        str += "<div class=\"col\">" + (i + 1).toString() + "</div>";
        
        str += "<div class=\"col\">";
        str += "<div class=\"container\" style=\"width: 100%\">"
        won = 0;        
        my_id = 0;
        for (j = 0; j < games[i].players.length; j++) {
            str += "<div class=\"table-row\" style=\"text-align: left; height: 24px;\">";             
            id = false;
            if (games[i].players[j].player_id == player_id) {
                id = true;
                my_id = j;
            }
            if (id) {
                str += "<b>";
            }
            else {
                str += "<a class=\"mya\" href=\"./player_res.html?player_id=" + games[i].players[j].player_id + "\">";
            }

            diff = games[i].players[j].new_rating - games[i].players[j].old_rating;
            pic = (diff >= 0 ? "up" : "down");
            diff = (diff > 0 ? "+" : "") + diff.toString();
            str += games[i].players[j].name + " (" + games[i].players[j].old_rating.toString() + ")";
            if (id) {
                str += "</b>";
            }
            else {
                str += "</a>";
            }
            str += "<img src=\"./img/" + pic + ".png\" style=\"float: right;\"/>" + "<span style=\"float: right\">  " + diff + "</span>";            
            str += "</div>";

            if (games[i].players[j].won) {
                won = j;
            }
        }
        str += "</div>";
        str += "</div>";

        str += "<div class=\"col\">" + ratings[i][1].toString() + "</div>";

        str += "</div>";
                
        $("#tbl").append(str);

        //inner = inner.children()[0];
        wline = $("#tbl .container .table-row").slice(-4).eq(won);
        wline.css("color", "#22CC55");
        wline.find("a").css("color", "#22CC55");
        //$("#tbl .container a").slice(-4).eq(won).css("color", "#22CC55");

        if (my_id == won) {
            $("#tbl .col").last().css("background-color", "#22CC55");
        }
        else {
            $("#tbl .col").last().css("background-color", "#BB2211");
        }

    }

});
