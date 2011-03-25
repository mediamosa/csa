<?php
// $Id:  $

// TODO: copyright here.
?>
  var swfu;
  var jsTimer = false;
  var error_send = false; // Indicates whether swfUpload has already been stopped because of a form error
  var upload_complete = false; // All queued files uploaded?
  var queue_complete = 0; // contains number of queued and successfully uploaded images
  var count_failed_uploads = 0; // Number of failed uploads

  window.onload = function() {
    var settings = {
      flash_url : "<?php print $modulepath; ?>/swfupload/swfupload.swf",
      upload_url: "<?php print $uploadpath; ?>",  // Relative to the SWF file
      post_params: {"PHPSESSID" : "<?php print $sessionid; ?>"} ,
      file_post_name: "file",
      file_size_limit : "<?php print $maxfilesize; ?>",
      file_types : "<?php print $fileextensions; ?>",
      file_types_description : "All files",
      file_upload_limit : "<?php print $uploadlimit; ?>",
      file_queue_limit : "0",
      custom_settings : {
        progressTarget : "csaUploadProgress",
        cancelButtonId : "btnCancel",
        uploadButtonId : "startuploadbutton"
      },
      debug: <?php print $debug; ?>,

      // Button settings
      button_width: "36",
      button_height: "37",
      button_placeholder_id: "spanUploadButton",
      button_image_url: "<?php print $modulepath; ?>/swfupload/select_images.png",  // Relative to the Flash file
      button_cursor: SWFUpload.CURSOR.HAND,

      // The event handler functions are defined in handlers.js
      file_queued_handler : fileQueued,
      file_queue_error_handler : fileQueueError,
      file_dialog_complete_handler : fileDialogComplete,
      upload_start_handler : uploadStart,
      upload_progress_handler : uploadProgress,
      upload_error_handler : uploadError,
      upload_success_handler : uploadSuccess,
      upload_complete_handler : uploadComplete,
      queue_complete_handler : queueComplete,  // Queue plugin event

      // CSA
      csa_upload_id : "<?php print $csa_upload_id; ?>",
      csa_phpsession_id : "<?php print $sessionid; ?>"
    };

    swfu = new SWFUpload(settings);
  };

  function startUploadProcess() {
    // Copy the value of the collection to a hidden value.
    if ($("input[@name='radio_collection']:checked").val() == 'enter') {
      $("#csa_coll_id").val($("#edit-enter-collection").val());
    }
    else {
      $("#csa_coll_id").val($("#edit-select-collection").val());
    }

    // Start upload.
    swfu.startUpload();
  }
