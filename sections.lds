SECTIONS{
  .features :
    {
      _features_start = .;
      *(.features)
      _features_end = .;
    } 
}
