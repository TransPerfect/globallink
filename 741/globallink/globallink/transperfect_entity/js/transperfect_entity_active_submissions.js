(function($) {
  Drupal.behaviors.transperfectEntityActiveSubmissions = {
    attach: function (context, settings) {
      if ($('.transperfect-entity-active-select-form', context).length == 0) {
        return;
      }

      $('.transperfect-entity-select-active-submission', context).change(function() {
        $('.transperfect-entity-active-select-form').submit();
      });
    }
  };
})(jQuery);
