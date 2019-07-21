var pref_layout, pref_data;


function onClosePopup (popup) {
    $(popup).css('display', 'none'); 
}

function onAcceptPopup (popup, input, key) {
    $('#' + key).text($(input).val());

    onClosePopup(popup);
}

function editNumber(context, value) {
    $('#id-popup-number-title').text(context.name);
    $('#id-popup-number-input').val(value);
    $('#id-popup-number-close').attr('onclick', 'onClosePopup("#id-popup-number-edit")');
    $('#id-popup-number-accept').attr('onclick', 'onAcceptPopup("#id-popup-number-edit", "#id-popup-number-input","' + context.key + '")');


    $('#id-popup-number-edit').css('display', 'block');

    /*
    $('#id-popup-number-close').click(function() {
        $('#id-popup-number-edit').css('display', 'none');    
    });

    $('#id-popup-number-accept').click(function() {
        $('#id-popup-number-edit').css('display', 'none');    

        $('#' + context.key).text($('#id-popup-number-input').val());
        console.log('update value of ', context.key);
    }); 
    */   
}

function editString(context, value) {
    $('#id-popup-string-title').text(context.name);
    $('#id-popup-string-input').val(value);
    $('#id-popup-string-input').attr('maxlength', context.max);
    $('#id-popup-string-close').attr('onclick', 'onClosePopup("#id-popup-string-edit")');
    $('#id-popup-string-accept').attr('onclick', 'onAcceptPopup("#id-popup-string-edit", "#id-popup-string-input","' + context.key + '")');


    $('#id-popup-string-edit').css('display', 'block');
}

function editSelect(context, value) {
    $('#id-popup-select-title').text(context.name);

    var _group = $('#id-popup-select-group');
    
    _group.text('');
    $.each(context.list, function (idx, name) {
        var _input = $('<input>')
            .attr('id', 'id-popup-radio-' + context.key)
            .addClass('popup-radio')
            .attr('type', 'radio')
            .attr('name', 'group');
        var _span = $('<span>').text(name);
        
        _group.append(_input);
        _group.append(_span);
        _group.append($('<br>'));
    });  


    $('#id-popup-select-close').attr('onclick', 'onClosePopup("#id-popup-select")');
    $('#id-popup-select-accept').attr('onclick', 'onAcceptPopup("#id-popup-select", "#id-popup-select-input","' + context.key + '")');

    $('#id-popup-select').css('display', 'block');
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
        .attr('context', JSON.stringify(item));

    if (item.type === "boolean") {
        _value.append($('<span>').text(item.desc));
        _value.append($('<span>').append($('<i>').addClass('fas fa-toggle-on').attr('style','position: absolute; right:0px; width:60px')));
    } else {
        _value.attr('onclick', 'editPrefItem(this)').text(pref_data[item.key]);

        if (item.type == "select") {
            //_value.text(item.list[pref_data[item.key]]);
            _value.text(item.list[_value.text()]);
        }
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

       $("#vario_pref").text("");

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

function savePref () {
    console.log("save preferences...");
}