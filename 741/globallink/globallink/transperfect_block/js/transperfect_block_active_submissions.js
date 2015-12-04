(function($) {
  Drupal.behaviors.transperfectBlockActiveSubmissions = {
    attach: function(context, settings) {
      if ($('.transperfect-block-active-select-form', context).length == 0) {
        return;
      }

      $('.transperfect-block-select-active-submission', context).change(function() {
        $('.transperfect-block-active-select-form').submit();
      });
    }
  };
})(jQuery);
