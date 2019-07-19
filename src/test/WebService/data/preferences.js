var pref_layout, pref_data;


function editNumber(context, value) {
    $('#id-popup-edit-number').css('display', 'block');
    $('#id-popup-number-input').value = value;

    $('#id-popup-number-close').click(function() {
        $('#id-popup-edit-number').css('display', 'none');    
    });

    $('#id-popup-number-accept').click(function() {
        $('#id-popup-edit-number').css('display', 'none');    


    });    
}

function editString(context, value) {

}

function editSelect(context, value) {

}

function editPrefItem(item) {
    var context = JSON.parse($(item).attr('context'));
    console.log('context = ', context);
    console.log('value = ', $(item).text());

    if (context.type === 'number') {
        editNumber(context, $(item).text());
    } else if (context.type === 'string') {
        editString(context, $(item).text());
    } else if (context.type === 'select') {
        editSelect(context, $(item).text());
    } else if (context.type === 'boolean') {

    }
}

function makePrefItem (item) {
    // { type, name, desc, key }

    var _hr = $('<hr>').addClass('pref-seperator');
    var _title = $('<div>')
        .addClass('pref-title')
        .text(item.name);
    var _value = $('<div>')
        .addClass('pref-value')
        .attr('id', item.key)
        .attr('context', JSON.stringify(item))
        .attr('onclick', 'editPrefItem(this)')
        .text(pref_data[item.key]);

    if (item.type == "select") {
        //_value.text(item.list[pref_data[item.key]]);
        _value.text(item.list[_value.text()]);
    }

    return $('<div>')
        .append(_hr)
        .append(_title)
        .append(_value);

    /*
    var prefItem =
        '<div>' +
            '<hr class="pref-separator"/>' +
            '<div class="pref-title">' + item.name + '</div>' +
            '<div class="pref-value">' + pref_data[item.key] + '</div>' +
        '</div>';

    return prefItem;
    */
}

/*
function makePrefItems (parent, items) {
    var prefItems = "";

    $.each(items, function (idx, item) {
        console.log(idx, item);
        prefItems += makePrefItem(item);
    });

    return prefItems;
}
*/

function makePrefGroup (title, items) {
    /*
    var group = 
        '<div>' +
            '<h1 class="pref-group">' + title + '</h1>' +
            makePrefItems(items) +
        '</div>';

    return group;
    */

    var _group = $('<div>');
    _group.append($('<h1>').addClass('pref-group').text(title));

    $.each(items, function (idx, item) {
        console.log(idx, item);
        _group.append(makePrefItem(item));
    });    

    return _group;
}
	
function makePreference () {
    if (pref_layout != "" && pref_data != "") {
        /*
        var pref = "";

        console.log("makePreference: ", pref_layout);
        $.each(pref_layout, function(key, value) {
            pref += makePrefGroup (value.title, value.items);
        });

        $("#vario_pref").html(pref);
        */

       console.log("makePreference: ", pref_layout);
       $.each(pref_layout, function(key, value) {
            $("#vario_pref").append(makePrefGroup (value.title, value.items));
       });
    }
}

function refreshPage () {
    //
    pref_layout = "";
    pref_data = "";

    //
    $.getJSON("pref-layout.json", function (data) {
        pref_layout = data; // JSON.parse(data);
        console.log("pref-laytout.json: ", data);
        makePreference();
    });	
    
    $.getJSON("pref-data.json", function (data) {
        pref_data = data; // JSON.parse(data);
        console.log("pref-data.json: ", data);
        makePreference();
    });	    
}