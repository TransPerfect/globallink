var _div;
var _img;
(function ($) {
    Drupal.behaviors.transperfect =   {
        attach: function() {
            if(Drupal.settings.transperfect != undefined) {
                var popup = Drupal.settings.transperfect.popup;
                var previewpath = Drupal.settings.transperfect.previewpath;
                var rids = Drupal.settings.transperfect.rids;
                _img = Drupal.settings.transperfect.ajax_image;
                $.each(popup, function(link, div) {
                    //Attach click event and set the contents of div
                    $('#' + link).click(function() {
                        _div = div;
                        $('#' + div).dialog({
                            modal: true,
                            show: {
                                effect: "blind",
                                duration: 100
                            },
                            width: 700,
                            height : 400
                        });
                        $('#' + div).empty();
                        $('#' + div).append('<div style="width: 100%; height:100%; text-align:center;"><div class="ajax-progress ajax-progress-throbber"><div class="throbber">&nbsp;</div></div></div>');
                        $.ajax({
                            type: 'POST',
                            url: previewpath,
                            dataType: 'json',
                            data: 'rid=' + rids[div],
                            success: ajaxCompleted,
                            error: function(xhr, textStatus, errorThrown) {
                                $('#' + _div).empty();
                                $('#'+div).html(textStatus);
                            }
                        });
                    });
                });
            }
        }
    }
})(jQuery);

function escapeHTML( string )
{
    return jQuery( '<pre>' ).text( string ).html();
}

function ajaxCompleted ($data) {
    var content = '<TABLE class="tpt_popup_table"><TR><TH>&nbsp;</TH><TH>Source Content</TH><TH>Translated Content</TH></TR>';
    error = $data['error'];
    target = $data['target'];
    source_obj = $data['source'];
    if(error != null && error != undefined) {
        content += '<TR><TD colspan="3"><span style="color: red;text-align: center;">' + error + '</span></TD></TR>';
        content += '</TABLE>';
        jQuery('#' + _div).empty();
        jQuery('#' + _div).append(content);
        return true;
    }
    if(source_obj == null || source_obj == undefined || source_obj == '') {
        return true;
    }
    jQuery.each(target, function(field, f_object) {
            if(field == 'link_title') {
                var source_text = '';
                var target_text = '';
                if(source_obj[field] != null && source_obj[field] != undefined) {
                    source_text = escapeHTML(source_obj[field]);
                }
                if(f_object != null && f_object != undefined) {
                    target_text = escapeHTML(f_object);
                    if(target_text != '') {
                        if(field == 'link_title') {
                            label = 'Title';
                        }
                        content += '<TR><TD><b>' + label + '</b></TD><TD>' + source_text + '</TD><TD>' + target_text + '</TD></TR>';
                    }
                }
            }
         else if(field == 'title') {
            var source_text = '';
            if(source_obj[field] != null && source_obj[field] != undefined) {
                source_text = escapeHTML(source_obj[field]);
            }           
            if(source_text == '') {
                source_text = '<span style="color:red;">Field Empty</span>';
            }
            if(f_object != null && f_object != undefined) {
                target_text = escapeHTML(f_object);
                if(target_text != '') {
                    if(field == 'title') {
                        label = 'Description';
                    }
                    content += '<TR><TD><b>' + label + '</b></TD><TD>' + source_text + '</TD><TD>' + target_text + '</TD></TR>';
                }
            }
        }
    });

    content += '</TABLE>';
    jQuery('#' + _div).empty();
    jQuery('#' + _div).append(content);

    return true;
}