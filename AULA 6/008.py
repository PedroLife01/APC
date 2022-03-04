def hh_duracao(hora_inicio,minuto_inicio,segundo_inicio,hora_final,minuto_final,segundo_final):
    SegundosTotalInicio = (hora_inicio*3600)+(minuto_inicio*60)+(segundo_inicio)
    SegundosTotalFinal = (hora_final*3600)+(minuto_final*60)+(segundo_final)

    if SegundosTotalInicio>SegundosTotalFinal:
        SegundoTotal = (24*3600) - (SegundosTotalInicio - SegundosTotalFinal)
        TempoHora = SegundoTotal//3600
        TempoMinuto = (SegundoTotal%3600)//60
        TempoSegundo = (SegundoTotal%3600)%60
        return (TempoHora,TempoMinuto,TempoSegundo)
    elif SegundosTotalInicio==SegundosTotalFinal:
        SegundoTotal = (24 * 3600)
        TempoHora = SegundoTotal//3600
        TempoMinuto = (SegundoTotal%3600)//60
        TempoSegundo = (SegundoTotal%3600)%60
        return (TempoHora, TempoMinuto, TempoSegundo)
    else:
        SegundoTotal = (SegundosTotalFinal) - (SegundosTotalInicio)
        TempoHora = SegundoTotal//3600
        TempoMinuto = (SegundoTotal%3600)//60
        TempoSegundo = (SegundoTotal%3600)%60
        return (TempoHora, TempoMinuto, TempoSegundo)

