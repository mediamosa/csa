/* $Id: $ */

/* 
	File: csa_upload.js
*/

function csa_click_radio_collection() {
	if ($("input[@name='radio_collection']:checked").val() == 'enter') {
		$("#edit-enter-collection").focus();
	}
}

function csa_click_enter_collection() {
	$("#edit-radio-collection-enter").attr('checked', true);
}

function csa_click_select_collection() {
	$("#edit-radio-collection-select").attr('checked', true);
}
