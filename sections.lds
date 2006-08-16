SECTIONS{
  .data :
    {
      _features_start = .;
      *(.features)
      _features_end = .;
      *(.data)
    } 
}
