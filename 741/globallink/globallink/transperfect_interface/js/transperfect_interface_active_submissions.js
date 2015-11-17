(function($) {
  Drupal.behaviors.transperfect = {
    attach: function (context, settings) {
      if ($('.transperfect-interface-active-select-form', context).length == 0) {
        return;
      }

      $('.transperfect-interface-select-active-submission', context).change(function() {
        $('.transperfect-interface-active-select-form').submit();
      });
    }
  };
})(jQuery);
