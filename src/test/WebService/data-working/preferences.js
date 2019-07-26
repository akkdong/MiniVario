/*
 *
 *
 *
 *
 *
 *
 */

var pref_layout, pref_data;


//
//
//

function onClickCancel () {
    $('#id-popup-editor').css('display', 'none');
}

function onClickAccept () {
    //
    var _context = JSON.parse($('#id-popup-editor').attr('context'));

    if (_context.type === 'select') {
        // id-popup-select, select-xxx_xxx_xxx, xxx_xxx_xxx
        var _select = $('input[name=select-'+_context.key+']:checked').val();
        var _option = _context.options.find(function (option) {
            return (option.value == _select);
        });

        pref_data[_context.key] = _option.value;
        $('#' + _context.key).text(_option.name);
    } else {
        var _ivalue = $('#id-popup-input').val();
        if (_context.type === 'number')
            _ivalue = Number(_ivalue);

        $('#' + _context.key).text(_ivalue);
        pref_data[_context.key] = _ivalue;
    }
    console.log("pref_data = ", JSON.stringify(pref_data));

    //
    onClickCancel();
}

function editItem (item) {
    var _context = JSON.parse($(item).attr('context'));
    var _value = $(item).text();
    var _body = $('#id-popup-body');
    console.log('context = ', _context);
    console.log('value = ', _value);

    $('#id-popup-title').text(_context.name);

    if (_context.type === 'number') {
        var _input = $('<input>').attr('id', 'id-popup-input').attr('type', 'number').addClass('popup-input');
        var _hr = $('<hr>').addClass('popup-input-inline');

        _input.val(_value);
        
        _body.empty();
        _body.append(_input);
        _body.append(_hr);
    } else if (_context.type === 'string') {
        var _input = $('<input>').attr('id', 'id-popup-input').attr('type', 'string').addClass('popup-input');
        var _hr = $('<hr>').addClass('popup-input-inline');
        
        _input.val(_value);
        _input.attr('maxlength', _context.maxlength);
        
        _body.empty();
        _body.append(_input);
        _body.append(_hr);
    } else if (_context.type === 'select') {
        var _group = $('<div>').attr('id', 'id-select-group');
    
        $.each(_context.options, function (idx, option) {
            var _div = $('<div>').css('font-size', '0.8em').css('color', 'black');
            var _input = $('<input>')
                .attr('id', 'item-' + idx)
                .addClass('popup-radio')
                .attr('type', 'radio')
                .attr('value', option.value)
                .attr('name', 'select-' + _context.key);
            var _span = $('<label>').text(option.name).attr('for', 'item-' + idx);
    
            if (option.name === _value)
                _input.attr('checked', 'checked');
            
            _div.append(_input);
            _div.append(_span);
            _group.append(_div);
        });

        _body.empty();
        _body.append(_group);
    }

    // remove & re-assign click handler
    $('#id-popup-accept').off('click');
    $('#id-popup-accept').click(onClickAccept);
    /*
    $('#id-popup-accept').click(function () {
        $('#id-popup-editor').css('display', 'none');    

        if (_context.type === 'select') {
            // id-popup-select, select-xxx_xxx_xxx, xxx_xxx_xxx
            var _select = $('input[name=select-'+_context.key+']:checked').val();
            var _option = _context.options.find(function (option) {
                return (option.value == _select);
            });

            pref_data[_context.key] = _option.value;
            $('#' + _context.key).text(_option.name);
        } else {
            var _input_val = $('#id-popup-input').val();
            if (_context.type === 'number')
                _input_val = Number(_input_val);

            console.log('input val =', _input_val);
            $('#' + _context.key).text(_input_val);
            pref_data[_context.key] = _input_val;
        }

        console.log("pref_data = ", JSON.stringify(pref_data));
    });
    */

    // remove & re-assign click handler
    $('#id-popup-close').off('click');
    $('#id-popup-close').click(onClickCancel);
    /*
    $('#id-popup-close').click(function () {  
        $('#id-popup-editor').css('display', 'none');
    });
    */

    // show popup
    $('#id-popup-editor').attr('context', JSON.stringify(_context));
    $('#id-popup-editor').css('display', 'block');

    if (_context.type === 'select') {
        $('input[name=select-'+_context.key+']:checked').focus();
    } else {
        $('#id-popup-input').focus().select();
    }
}

function makePrefItem (item) {
    // item: { type, name[, desc,] key[, min, max, step, maxlength, options] }

    var _hr = $('<hr>').addClass('pref-seperator');
    var _title = $('<div>')
        .addClass('pref-title')
        .text(item.name);
    var _value = $('<div>')
        .addClass('pref-value')
        .attr('id', item.key)
        .attr('context', JSON.stringify(item));

    if (item.type === "boolean") {
        var _toggle = $('<i>').attr('style','position: absolute; right:8px;')
        if (pref_data[item.key]) {
            _toggle.addClass('fas fa-toggle-on');
            _toggle.css('color', 'green');
        } else {
            _toggle.addClass('fas fa-toggle-off')
            _toggle.css('color', 'gray');
        }
        _value.click(function () {
            pref_data[item.key] = !!! pref_data[item.key];
            console.log('pref_data = ', JSON.stringify(pref_data));

            if (pref_data[item.key]) {
                _toggle.attr('class', 'fas fa-toggle-on');
                _toggle.css('color', 'green');
            } else {
                _toggle.attr('class', 'fas fa-toggle-off')
                _toggle.css('color', 'gray');
            }
        });
        _value.append($('<span>').text(item.desc));
        _value.append($('<span>').append(_toggle));
    } else if (item.type === "select-inline") {
        var _select = $('<select>').attr('id', 'id-select-inline-' + item.key);

        $.each(item.options, function (idx, option) {
            var _option = $('<option>').val(option.name).text(option.name);
            if (option.value == pref_data[item.key])
                _option.attr('selected', 'selected');

            _select.append(_option);
        });

        _select.change(function() {
            var _name = $(this).children('option:selected').text();
            console.log("timezone changed: ", _name);

            pref_data[item.key] = item.options.find(function (option) {
                return (_name == option.name);
            }).value;
            console.log('pref_data = ', JSON.stringify(pref_data));
        });

        _value.append(_select);
    } else {
        _value.attr('onclick', 'editItem(this)').text(pref_data[item.key]);

        if (item.type === "select") {
            //_value.text(item.list[pref_data[item.key]]);
            var _option = item.options.find(function (option) {
                return (_value.text() == option.value);
            });
            _value.text(_option.name);
        }
    }

    return $('<div>')
        .append(_hr)
        .append(_title)
        .append(_value);
}

function makePrefGroup (title, items) {
    var _group = $('<div>').append($('<div>').addClass('pref-group').text(title));

    $.each(items, function (idx, item) {
        _group.append(makePrefItem(item));
    });    

    return _group;
}
	
function makePreference () {
    if (pref_layout != "" && pref_data != "") {
       console.log("makePreference: ", pref_layout);       
       // empty
       $("#vario_pref").empty();
       // append each groups
       $.each(pref_layout, function(key, value) {
            $("#vario_pref").append(makePrefGroup (value.title, value.items));
       });
    }
}

function refreshPage () {

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

function fetch_PrefData() {
    $.getJSON("pref-data.json", function (data) {
        pref_data = data; // JSON.parse(data);
		console.log("pref-data.json: ", data);
		
        makePreference();
    });	    
}

function fetch_PrefLayout() {
    $.getJSON("pref-layout.json", function (data) {
        pref_layout = data; // JSON.parse(data);
        console.log("pref-laytout.json: ", data);
		
		fetch_PrefData();
    });	
}

function initPref() {
    //
    pref_layout = "";
	pref_data = "";

	fetch_PrefLayout();

    //
    $("#id-download").click(function () {
        savePref();
    });

    $("#id-reload").click(function () {
        refreshPage();
    });

    //
    $(document).on( "keydown", function (evt) {
        if (evt.keyCode == 13) {
            if ($('#id-popup-editor').css('display') === 'block') {
                onClickAccept();
            }
        } else if (evt.keyCode == 27) {
            if ($('#id-popup-editor').css('display') === 'block') {
                onClickCancel();
            }
        }
    });    
}

function savePref () {
    console.log("save preferences...");
    $.post("/update/pref-data.json", pref_data, function (data) {
        console.log(data);
    }, "json");
}
