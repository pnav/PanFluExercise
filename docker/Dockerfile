FROM pnav/panflu-exercise-base:latest

USER panflu:panflu
ENV PANFLU_ROOT /home/panflu
ENV PANFLU_DATA ${PANFLU_ROOT}/data
ENV PANFLU_UTIL ${PANFLU_ROOT}/util
ENV PANFLU_BIN  ${PANFLU_ROOT}/bin

ENTRYPOINT ["/home/panflu/bin/exercise"]

CMD [ "--batch-numtimesteps", "450" \
      "--batch-initialcasesfilename", "${PANFLU_UTIL}/initialcases.xml" \
      "--batch-parametersfilename", "${PANFLU_UTIL}/parameters.xml" \
      "--batch-outputvariable", "deceased" \
      "--batch-outputfilename", "${PANFLU_DATA}/deceased-18.csv" ]



