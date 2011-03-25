// $Id$

(function ($) {

  /**
   * Prevent pathauto from taking over our custom alias by
   * removing the checkbox for its setting if a custom path
   * was entered before.
   */
  Drupal.behaviors.preventPathauto = function() {
    if($('#edit-path').val()) {
      if ($('#edit-path').val().length > 0) {
        $('#edit-pathauto-perform-alias').attr('checked', '');
        $('#edit-path').removeAttr('disabled');
      }
    }
  }

  /**
   * Load superfish menu when its class is set on one of the
   * parent items of the menu list.
   */
  Drupal.behaviors.loadSuperfish = function() {
    $('.superfish ul.menu').superfish();
  };

  /**
   * Catch tabs-lists that are too long and add paging.
   */
  Drupal.behaviors.hideTabs = function(context) {
    var tabindex  = 0;
    var tabsize   = 0;
    var max_size  = $('div.tabs').outerWidth();

    $('div.tabs ul.primary li a').once(function(index) {
      var tab_width = $(this).outerWidth(true);
      tabsize += tab_width;
      if (tabsize > max_size && tabindex == 0) {
        tabindex = index - 1; // remember the first tab that would be wider then the available space
      }
    });

    if (tabindex != 0) {
      $('div.tabs ul.primary li').slice(0, tabindex - 1).addClass('main-active');

      /**
       * Hide all the tabs, except the main tabs, and the active tabs
       */
      $('div.tabs ul.primary li:not(.main-active):not(.active)').hide();

      /**
       * Create new tab to switch between active and non-active tabs
       */
      var new_tab = $('div.tabs ul.primary li:first')
        .clone()
        .removeClass('active main-active')
        .addClass('tab_switcher')
        .appendTo('div.tabs ul.primary');
      $('a', new_tab)
        .html('>')
        .attr('href', '#')
        .attr('title', Drupal.t('More tabs'))
        .click(function() {
          $('div.tabs ul.primary li:not(.tab_switcher)').toggle();
          if ($(this).html() == '&gt;') {
            $(this).html('<');
            $(this).parent().remove('true').prependTo('div.tabs ul.primary');
          }
          else {
            $(this).html('>');
            $(this).parent().remove('true').appendTo('div.tabs ul.primary');
          }
          return false; })
        .removeClass('active main-active');
    }
  };

})(jQuery);
