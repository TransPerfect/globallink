(function($) {
  Drupal.behaviors.transperfectWebformActiveSubmissions = {
    attach: function(context, settings) {
      if ($('.transperfect-webform-active-select-form', context).length == 0) {
        return;
      }

      $('.transperfect-webform-select-active-submission', context).change(function() {
        $('.transperfect-webform-active-select-form').submit();
      });
    }
  };
})(jQuery);
