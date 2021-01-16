 val = ADD( val, g.shfl_down( val, i) );


    t = g.shfl_down( val, i) ;
    val = ADD( val, t );

    GB_ADD (val, t) ;           // statment  val = GB_ADD_FUNCTION (val, t)


