/**
 * CSA is open source Software.
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * CSA is developed for the open source Drupal platform (http://drupal.org).
 * CSA has been developed by Madcap BV (http://www.madcap.nl).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

function csa_box_selectitem(obj, type, item_id) {

  if ($(obj).is(':checked')) {
    //  Make sure we select the item and we know.
    $.ajax({
      url: Drupal.settings.basePath + 'csa/box/selectitem/' + $.URLEncode(type) + '/' + $.URLEncode(item_id),
      async: false,
      dataType: 'json',
      success: function(data) {
      }
    });  
  }
  else {
    //  Make sure we select the item and we know.
    $.ajax({
      url: Drupal.settings.basePath + 'csa/box/unselectitem/' + $.URLEncode(type) + '/' + $.URLEncode(item_id),
      async: false,
      dataType: 'json',
      success: function(data) { 
      }
    });
  }

  // Update possible text.
  csa_box_update_selection_text(type);
}

function csa_box_checkbox_toggle_all(obj, type, $selector) {
  var csa_box_checkbox_toggle_status = $(obj).is(':checked');

  var items = [];
  $($selector).each(function() {
    this.checked = csa_box_checkbox_toggle_status;
  
    items.push($.URLEncode($(this).attr('name')));
  });

  if ($(items).length) {
    if (csa_box_checkbox_toggle_status) {
      csa_box_selectitemall(type, items)
    }
    else {
      csa_box_unselectitemall(type, items)
    }
  }
}

function csa_box_selectitemall(type, items) {

  $.ajax({
    url: Drupal.settings.basePath + 'csa/box/selectitemall/' + $.URLEncode(type),
    async: false,
    dataType: 'json',
    type: 'post',
    data: {items : items.join('|')},
    success: function(data) {
    }
  });  

  // Update possible text.
  csa_box_update_selection_text(type);
}

function csa_box_unselectitemall(type, items) {

  $.ajax({
    url: Drupal.settings.basePath + 'csa/box/unselectitemall/' + $.URLEncode(type),
    async: false,
    dataType: 'json',
    type: 'post',
    data: {items : items.join('|')},
    success: function(data) {
    }
  });  

  // Update possible text.
  csa_box_update_selection_text(type);
}

function csa_box_update_selection_text(type) {

  try {
      // Update the quota text.
    var csaselectedtext = document.getElementById("csa_selected_text");
      if (csaselectedtext) {
        var result_text = [];
        
        $.ajax({
          url: Drupal.settings.basePath + 'csa/box/get_selected_text/' + $.URLEncode(type),
          async: false,
          dataType: 'json',
          success: function (data) {
            var csaselectedtext = document.getElementById("csa_selected_text");
              if (csaselectedtext) {
                csaselectedtext.innerHTML = data;
              }
          }
        });
      }
    } catch (ex) {
        // Doing nothing
    }    
  
}

function csa_submit_search(event) {
  if (event.keyCode == 13){
    $("#edit-search").click();
  }
}
