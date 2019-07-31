/*
 *
 *
 *
 *
 *
 *
 */

var pref_layout, pref_config;


var image_download = "data:image/png;base64,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";
var image_upload = "data:image/png;base64,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";


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

        pref_config[_context.key] = _option.value;
        $('#' + _context.key).text(_option.name);
    } else {
        var _ivalue = $('#id-popup-input').val();
        if (_context.type === 'number')
            _ivalue = Number(_ivalue);

        $('#' + _context.key).text(_ivalue);
        pref_config[_context.key] = _ivalue;
    }
    console.log("pref_config = ", JSON.stringify(pref_config));

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
        var _hr = $('<hr>').addClass('popup-input-line');

        _input.val(_value);
        
        _body.empty();
        _body.append(_input);
        _body.append(_hr);
    } else if (_context.type === 'string') {
        var _input = $('<input>').attr('id', 'id-popup-input').attr('type', 'string').addClass('popup-input');
        var _hr = $('<hr>').addClass('popup-input-line');
        
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

    // remove & re-assign click handler
    $('#id-popup-close').off('click');
    $('#id-popup-close').click(onClickCancel);

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

    var _hr = $('<hr>').addClass('pref-separator');
    var _title = $('<div>')
        .addClass('pref-title')
        .text(item.name);
    var _value = $('<div>')
        .addClass('pref-value')
        .attr('id', item.key)
        .attr('context', JSON.stringify(item));

    if (item.type === "boolean") {
        /*
        var _toggle = $('<i>').attr('style','position: absolute; right:8px;')
        if (pref_config[item.key]) {
            _toggle.addClass('fas fa-toggle-on');
            _toggle.css('color', 'green');
        } else {
            _toggle.addClass('fas fa-toggle-off')
            _toggle.css('color', 'gray');
        }
        _value.click(function () {
            pref_config[item.key] = !!! pref_config[item.key];
            console.log('pref_config = ', JSON.stringify(pref_config));

            if (pref_config[item.key]) {
                _toggle.attr('class', 'fas fa-toggle-on');
                _toggle.css('color', 'green');
            } else {
                _toggle.attr('class', 'fas fa-toggle-off')
                _toggle.css('color', 'gray');
            }
        });
        _value.append($('<span>').text(item.desc));
        _value.append($('<span>').append(_toggle));
        */
       var _label = $('<label>', { class: 'switch' });
       var _input = $('<input>', { type: 'checkbox', disabled: 'disabled' });
       if (pref_config[item.key]) {
           _input.attr('checked', 'checked');
       }
       _label.append(_input);
       _label.append($('<span>', { class: 'slider round'}));
       
       _value.append($('<span>').text(item.desc));
       _value.append($('<span>', { style: 'position: absolute; right:8px;'}).append(_label));

       _value.click(function () {
        pref_config[item.key] = !!! pref_config[item.key];
           console.log('pref_config = ', JSON.stringify(pref_config));

           if (pref_config[item.key]) {
               _input.attr('checked', 'checked');
           } else {
               _input.removeAttr('checked');
           }
       });
    } else if (item.type === "select-inline") {
        var _select = $('<select>').attr('id', 'id-select-inline-' + item.key);

        $.each(item.options, function (idx, option) {
            var _option = $('<option>').val(option.name).text(option.name);
            if (option.value == pref_config[item.key])
                _option.attr('selected', 'selected');

            _select.append(_option);
        });

        _select.change(function() {
            var _name = $(this).children('option:selected').text();
            console.log("timezone changed: ", _name);

            pref_config[item.key] = item.options.find(function (option) {
                return (_name == option.name);
            }).value;
            console.log('pref_config = ', JSON.stringify(pref_config));
        });

        _value.append(_select);
    } else {
        _value.attr('onclick', 'editItem(this)').text(pref_config[item.key]);

        if (item.type === "select") {
            //_value.text(item.list[pref_config[item.key]]);
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
    if (pref_layout != "" && pref_config != "") {
       console.log("makePreference: ", pref_layout);       
       // empty
       $("#vario_pref").empty();
       // append each groups
       $.each(pref_layout, function(key, value) {
            $("#vario_pref").append(makePrefGroup (value.title, value.items));
       });
    }
}

function fetch_PrefData() {
    $.getJSON("config.json", function (data) {
        pref_config = data; // JSON.parse(data);
		console.log("config.json: ", data);
		
        makePreference();
    });	    
}

function fetch_PrefLayout() {
    $.getJSON("layout.json", function (data) {
        pref_layout = data; // JSON.parse(data);
        console.log("laytout.json: ", data);
		
		fetch_PrefData();
    });	
}

function initPref() {
    //
    pref_layout = "";
	pref_config = "";

	fetch_PrefLayout();

    //
    $("#id-download").click(function () {
		if (confirm("Save Preferences?") == true) {
        savePref();
		}
    });

    $("#id-reload").click(function () {
		if (confirm("Reload Preferences?") == true) {
			$("#id-download").off("click");
			$("#id-reload").off("click");
			$(document).off("keydown");
			
			initPref();
		}
    });

    $("#id-download img").attr("src", image_download );
    $("#id-reload img").attr("src", image_upload );
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
    $.post("/Update/config.json", pref_config, function (data) {
        alert("Save Preferences Sucsess.");
    }).fail(function(xhr, stat, err) {
		alert("Save Preferences Failed!");
	});
}
