(function($) {
  Drupal.behaviors.transperfectFieldablePanelsActiveSubmissions = {
    attach: function(context, settings) {
      if ($('.transperfect-fieldable-panels-active-select-form', context).length == 0) {
        return;
      }

      $('.transperfect-fieldable-panels-select-active-submission', context).change(function() {
        $('.transperfect-fieldable-panels-active-select-form').submit();
      });
    }
  };
})(jQuery);
