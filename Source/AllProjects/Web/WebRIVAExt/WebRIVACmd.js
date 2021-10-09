//
//  You must provide support for any custom RIVA commands you want to use. The WebRIVA
//  program will handle any standard ones. Any others are passed to rivaDoRIVACmd()
//  below. Any parameters passed to the command will be passed in the p1 .. p4
//  parameters. Any not set by the invoking command will be empty values.
//
//  You can provide any other local helpers you need.
//
rivaDoRIVACmd = function(cmdPath, p1, p2, p3, p4) {
    console.log("RIVACmd: " + cmdPath + ", p1: " + p1 + ", p2: " + p2 + ", p3: " + p3);
}
