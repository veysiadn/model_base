/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include <TensorFlowLite.h>

#include "main_functions.h"

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "model.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
int inference_count = 0;

constexpr int kTensorArenaSize = 10*1024;
uint8_t tensor_arena[kTensorArenaSize];
float_t* model_input_buffer = nullptr;
}  // namespace




//
float example_signal[] = {
       -6.06968882e-04, -6.06968882e-04, -6.06968882e-04, -6.06968882e-04,
       -6.06968882e-04, -6.06968882e-04, -6.06968882e-04, -6.06968882e-04,
       -6.06968882e-04, -6.06968882e-04, -6.06968882e-04, -6.06968882e-04,
       -6.06968882e-04, -6.06968882e-04, -6.06968882e-04, -6.06968882e-04,
       -6.06968882e-04, -6.06968882e-04, -6.06968882e-04, -6.06968882e-04,
       -6.06968882e-04, -6.06968882e-04, -6.06968882e-04, -6.06968882e-04,
       -6.06968882e-04, -6.06968882e-04, -6.06968882e-04, -6.06968882e-04,
       -6.06968882e-04, -6.06968882e-04, -6.06968882e-04, -6.06968882e-04,
       -6.06968882e-04, -6.06968882e-04, -6.06968882e-04, -6.06968882e-04,
       -6.06968882e-04, -6.06968882e-04, -6.06968882e-04, -6.06968882e-04,
       -6.06968882e-04, -6.06968882e-04, -6.06968882e-04, -6.06968882e-04,
       -6.06968882e-04,  4.99256626e-02, -2.60837823e-02, -1.30357847e-01,
       -2.38039121e-01, -3.42974126e-01, -4.53352600e-01, -5.59207678e-01,
       -6.56093478e-01, -7.40052223e-01, -8.08209956e-01, -8.55113208e-01,
       -8.72139752e-01, -8.62764657e-01, -8.29253495e-01, -7.53657758e-01,
       -6.45975649e-01, -5.14317274e-01, -3.40258062e-01, -1.33946061e-01,
        8.18552226e-02,  2.94743091e-01,  5.14381886e-01,  7.42882073e-01,
        9.36185062e-01,  1.05214655e+00,  1.09983516e+00,  1.10304356e+00,
        1.05699134e+00,  9.25213873e-01,  7.07849562e-01,  4.38609719e-01,
        1.04416050e-01, -3.14447701e-01, -7.47235835e-01, -1.09551227e+00,
       -1.37981880e+00, -1.64777970e+00, -1.87020302e+00, -1.90136385e+00,
       -1.93377101e+00, -2.14208126e+00, -2.13688779e+00, -2.07881546e+00,
       -2.12485814e+00, -1.85949433e+00, -1.36804545e+00, -8.10079217e-01,
       -2.07765162e-01,  5.24326503e-01,  1.25686657e+00,  1.82767713e+00,
        2.26517010e+00,  2.67048001e+00,  2.93478251e+00,  3.03484011e+00,
        3.01025534e+00,  2.88554072e+00,  2.68444943e+00,  2.43037701e+00,
        2.07947707e+00,  1.59245491e+00,  1.03317070e+00,  4.55395550e-01,
       -2.04147130e-01, -9.33063030e-01, -1.62986684e+00, -2.20990753e+00,
       -2.65856743e+00, -3.01335740e+00, -3.27329516e+00, -3.42670941e+00,
       -3.50231981e+00, -3.48456931e+00, -3.34708595e+00, -3.05015898e+00,
       -2.55586672e+00, -1.88561678e+00, -1.08752108e+00, -2.31323913e-01,
        5.75936079e-01,  1.19776213e+00,  1.71678710e+00,  2.18677950e+00,
        2.67715859e+00,  3.16068006e+00,  3.45862675e+00,  3.75562191e+00,
        3.78073549e+00,  3.48432136e+00,  3.06031156e+00,  2.46141315e+00,
        1.66075015e+00,  7.50182152e-01, -8.72975737e-02, -7.72179842e-01,
       -1.56082845e+00, -2.32136941e+00, -2.68836832e+00, -2.90239310e+00,
       -3.10689354e+00, -3.18821192e+00, -3.15053225e+00, -3.04564309e+00,
       -2.89395261e+00, -2.68621540e+00, -2.36542010e+00, -1.90161908e+00,
       -1.29802167e+00, -6.13378763e-01,  8.09932202e-02,  7.26906121e-01,
        1.28318977e+00,  1.70736492e+00,  1.93360484e+00,  1.92909229e+00,
        1.82190716e+00,  1.73288238e+00,  1.65710986e+00,  1.41459131e+00,
        9.19305980e-01,  5.38528442e-01,  1.87844619e-01, -5.11537611e-01,
       -8.92781496e-01, -9.71003771e-01, -1.13950169e+00, -1.32033885e+00,
       -1.30150497e+00, -1.25986743e+00, -1.36117387e+00, -1.25612056e+00,
       -1.05071080e+00, -8.85985374e-01, -7.74295092e-01, -6.29416943e-01,
       -3.45598578e-01,  4.02447283e-02,  4.08723056e-01,  6.15151942e-01,
        6.48879588e-01,  7.40188956e-01,  6.87519550e-01,  3.86041820e-01,
        2.59441763e-01,  2.34276354e-01,  1.63979635e-01,  1.04607984e-01,
        1.23787530e-01,  1.51211947e-01,  5.68239018e-02,  2.77004931e-02,
       -7.80172348e-02, -2.15804145e-01, -2.78727561e-01, -3.26289982e-01,
       -4.80091631e-01, -5.39175034e-01, -4.60921705e-01, -3.66311640e-01,
       -2.90275753e-01, -1.48437262e-01, -4.64279577e-02, -7.85444900e-02,
       -7.91634098e-02, -5.81685714e-02, -7.65181929e-02, -7.82502443e-02,
       -4.06030305e-02,  4.27781022e-04,  6.85779154e-02,  1.86259940e-01,
        3.59834820e-01,  5.03077626e-01,  5.82138717e-01,  6.36800110e-01,
        6.46153748e-01,  5.48434079e-01,  4.32030022e-01,  3.86961251e-01,
        2.95860797e-01,  1.20023683e-01,  8.73394981e-02,  1.07788369e-01,
        1.76645964e-02, -5.37976250e-02, -7.31817707e-02, -1.41397387e-01,
       -2.50044554e-01, -3.44836146e-01, -4.21425611e-01, -4.87722129e-01,
       -5.59620798e-01, -6.17701292e-01, -6.38333321e-01, -6.23587668e-01,
       -5.74439228e-01, -4.85381484e-01, -3.61467242e-01, -2.19361186e-01,
       -8.61406997e-02,  1.50328958e-02,  8.47344548e-02,  1.50914416e-01,
        2.44433284e-01,  3.75036478e-01,  5.12927890e-01,  6.30627811e-01,
        7.49676585e-01,  8.30045342e-01,  7.45921135e-01,  5.14126778e-01,
        2.96142370e-01,  7.97234252e-02, -1.22637458e-01, -2.64050722e-01,
       -3.39029372e-01, -3.97057652e-01, -4.69776720e-01, -5.65737307e-01,
       -6.85890257e-01, -8.15350413e-01, -9.22124326e-01, -1.00111699e+00,
       -1.05682194e+00, -1.06991136e+00, -1.00397992e+00, -8.33658934e-01,
       -5.87286413e-01, -3.06565940e-01, -3.80406156e-02,  1.74232602e-01,
        3.15556437e-01,  4.12765265e-01,  5.07043660e-01,  6.53950930e-01,
        7.90145397e-01,  9.11177814e-01,  1.06645095e+00,  1.21216226e+00,
        1.29144311e+00,  1.26932538e+00,  1.13125563e+00,  9.04802680e-01,
        6.17278576e-01,  2.70697236e-01, -8.15304965e-02, -3.50942582e-01,
       -5.50599396e-01, -7.24439979e-01, -8.61876190e-01, -9.34826076e-01,
       -9.63774085e-01, -9.79288220e-01, -1.00374436e+00, -1.02279925e+00,
       -1.00873625e+00, -9.38057959e-01, -7.90519714e-01, -5.57482302e-01,
       -2.67775923e-01,  1.60631109e-02,  2.48130158e-01,  4.26132470e-01,
        5.79470158e-01,  6.94602549e-01,  7.48198032e-01,  7.38999128e-01,
        7.06411421e-01,  6.91305399e-01,  6.91390872e-01,  6.97897315e-01,
        7.10034668e-01,  7.29806900e-01,  7.52549291e-01,  7.41867542e-01,
        6.48257256e-01,  4.75026011e-01,  2.48915628e-01,  2.01371014e-02,
       -1.92283452e-01, -3.86581123e-01, -5.53093612e-01, -6.77744389e-01,
       -7.35606074e-01, -7.12735236e-01, -6.25085175e-01, -4.99300033e-01,
       -3.69331032e-01, -2.60623455e-01, -1.84273079e-01, -1.24179967e-01,
       -5.72272725e-02,  3.78699005e-02,  1.67206958e-01,  3.03152949e-01,
        4.19369251e-01,  5.06248355e-01,  5.70532322e-01,  6.15534663e-01,
        6.26142204e-01,  5.83836675e-01,  4.74946529e-01,  3.07297558e-01,
        1.09542511e-01, -8.50671306e-02, -2.29841083e-01, -3.06194603e-01,
       -3.23382914e-01, -3.06930959e-01, -3.02701175e-01, -3.37823153e-01,
       -4.10300553e-01, -4.90665436e-01, -5.46248376e-01, -5.53823590e-01,
       -4.85642642e-01, -3.32213223e-01, -1.13122128e-01,  1.35931715e-01,
        3.66461307e-01,  5.36589801e-01,  6.18122876e-01,  6.02348328e-01,
        5.17097116e-01,  4.11378473e-01,  3.16776574e-01,  2.34354600e-01,
        1.57577083e-01,  8.25628564e-02,  9.10016242e-03, -7.13592023e-02,
       -1.69333935e-01, -2.72089362e-01, -3.39835018e-01, -3.46707940e-01,
       -2.90697038e-01, -2.08839685e-01, -1.26275554e-01, -5.14471382e-02,
        5.66980336e-03,  3.12072448e-02,  1.71556119e-02, -2.34765280e-02,
       -6.70032874e-02, -8.79115835e-02, -6.55329823e-02, -5.44549013e-03,
        7.90425837e-02,  1.62312508e-01,  2.10020974e-01,  2.06644952e-01,
        1.46950990e-01,  3.98137718e-02, -8.70842412e-02, -1.93529725e-01,
       -2.29455605e-01, -2.28025720e-01, -2.37652004e-01, -2.05844551e-01,
       -1.76456302e-01, -1.95624053e-01, -2.10560113e-01, -1.89048856e-01,
       -1.40414611e-01, -4.93632704e-02,  1.09253697e-01,  2.94194728e-01,
        4.45092618e-01,  5.39404988e-01,  5.72764874e-01,  5.31298161e-01,
        4.15558696e-01,  2.56557882e-01,  9.60216522e-02, -2.49616615e-02,
       -9.14495066e-02, -1.17927067e-01, -1.30993053e-01, -1.52309805e-01,
       -1.94943115e-01, -2.59980232e-01, -3.30627412e-01, -3.85958314e-01,
       -4.14062738e-01, -4.09435660e-01, -3.64259213e-01, -2.99981922e-01,
       -2.44947091e-01, -2.16777816e-01, -2.17502341e-01, -2.39237279e-01,
       -2.72325337e-01, -2.95313507e-01, -2.83834964e-01, -2.30906382e-01,
       -1.41974539e-01, -3.58772874e-02,  7.75730908e-02,  1.91836849e-01,
        2.99226016e-01,  3.91911119e-01,  4.63541687e-01,  5.20369768e-01,
        5.65471768e-01,  5.76023102e-01,  5.53965092e-01,  5.29880583e-01,
        5.10172069e-01,  4.91502434e-01,  4.64507431e-01,  4.39088672e-01,
        4.27316159e-01,  4.08803552e-01,  3.74310553e-01,  3.34091395e-01,
        2.83478767e-01,  2.07196265e-01,  9.28675756e-02, -5.13149165e-02,
       -2.10642174e-01, -3.70239645e-01, -5.07270694e-01, -6.03977382e-01,
       -6.72481418e-01, -7.38407552e-01, -7.89900661e-01, -8.27964365e-01,
       -8.85399699e-01, -9.24318671e-01, -9.14933026e-01, -8.24119747e-01,
       -6.63691580e-01, -4.53644693e-01, -2.11107805e-01,  4.20146063e-02,
        2.83619940e-01,  4.91605520e-01,  6.45209789e-01,  7.37491667e-01,
        7.73198247e-01,  7.59399116e-01,  7.03674257e-01,  6.13605917e-01,
        4.95906502e-01,  3.56169939e-01,  1.99943528e-01,  3.23111787e-02,
       -1.43624321e-01, -3.25558484e-01, -5.09336412e-01, -6.84536278e-01,
       -8.39230776e-01, -9.61492240e-01, -1.03939283e+00, -1.06171715e+00,
       -1.02185154e+00, -9.22969580e-01, -7.82147706e-01, -6.17132723e-01,
       -4.40333545e-01, -2.55921930e-01, -6.52721003e-02,  1.33046493e-01,
        3.40713799e-01,  5.50906360e-01,  7.44285345e-01,  8.99952054e-01,
        1.00401759e+00,  1.04571986e+00,  1.01826262e+00,  9.31359053e-01,
        7.95190632e-01,  6.20322943e-01,  4.39832836e-01,  2.58379906e-01,
        8.91274288e-02, -5.26590981e-02, -1.95836052e-01, -3.38169634e-01,
       -4.62293983e-01, -5.54499805e-01, -6.02526426e-01, -5.96998215e-01,
       -5.35980344e-01, -4.31553185e-01, -3.04577798e-01, -1.80064946e-01,
       -7.47784972e-02,  3.07494448e-03,  4.86272313e-02,  6.45112470e-02,
        6.83444962e-02,  8.30314979e-02,  1.13260008e-01,  1.47774205e-01,
        1.96685627e-01,  2.41469219e-01,  2.21283615e-01,  1.51807666e-01,
        6.92285150e-02, -3.22077870e-02, -1.39134064e-01, -1.70945227e-01,
       -1.48202434e-01, -1.25443503e-01, -9.31375772e-02, -3.59843969e-02,
       -2.06768233e-02, -3.37301455e-02, -2.37459298e-02,  2.11851369e-03,
        1.17568960e-02,  4.61626500e-02,  9.38904583e-02,  1.07668608e-01,
        1.00737780e-01,  8.40536132e-02,  5.63142188e-02,  1.08958082e-02,
       -3.01264822e-02, -4.99285832e-02, -8.25247988e-02, -1.25087872e-01,
       -1.52360439e-01, -1.73231721e-01, -1.92698359e-01, -1.99932873e-01,
       -1.91059172e-01, -1.71598852e-01, -1.41852438e-01, -8.65363032e-02,
       -9.73548100e-04,  8.88172016e-02,  1.77817076e-01,  2.69082993e-01,
        3.31471920e-01,  3.59693199e-01,  3.58660847e-01,  3.27459306e-01,
        2.72836059e-01,  2.06861079e-01,  1.45096287e-01,  1.01713784e-01,
        7.56171569e-02,  4.70447727e-02, -4.07097070e-03, -8.60116184e-02,
       -1.92330033e-01, -3.09658706e-01, -4.15176213e-01, -4.80715811e-01,
       -5.04250944e-01, -4.96092618e-01, -4.60893512e-01, -3.94012481e-01,
       -3.04076612e-01, -2.02451423e-01, -9.97162089e-02, -3.31376120e-03,
        8.73763040e-02,  1.73401996e-01,  2.56552607e-01,  3.42418343e-01,
        4.41920489e-01,  5.44407308e-01,  6.25247121e-01,  6.64979696e-01,
        6.57518208e-01,  5.89993060e-01,  4.69983608e-01,  3.21966201e-01,
        1.61358923e-01, -1.32186608e-02, -1.89641312e-01, -3.63362461e-01,
       -5.30156016e-01, -6.89782679e-01, -8.25642049e-01, -9.54734087e-01,
       -1.05867410e+00, -1.12089455e+00, -1.15213525e+00, -1.14269030e+00,
       -1.06735861e+00, -9.35662687e-01, -7.64766216e-01, -5.62873006e-01,
       -3.58902007e-01, -1.76593155e-01, -4.40899003e-03,  1.65643871e-01,
        3.23619068e-01,  4.86880541e-01,  6.53059006e-01,  7.93294311e-01,
        9.01350856e-01,  9.79397893e-01,  1.00483632e+00,  9.67164397e-01,
        8.75770271e-01,  7.35658109e-01,  5.58149576e-01,  3.67970556e-01,
        1.67666689e-01, -3.78133617e-02, -2.29541376e-01, -4.07096773e-01,
       -5.80122888e-01, -7.50816047e-01, -9.01148915e-01, -1.02705669e+00,
       -1.11556995e+00, -1.13843513e+00, -1.10123682e+00, -1.01386297e+00,
       -8.87584627e-01, -7.38715947e-01, -5.81745684e-01, -4.23469037e-01,
       -2.73549110e-01, -1.27355650e-01,  4.45720553e-02,  2.47822866e-01,
        4.73518431e-01,  7.08760440e-01,  9.31326210e-01,  1.10317528e+00,
        1.18647921e+00,  1.21845710e+00,  1.18729997e+00,  1.10163069e+00,
        9.97316957e-01,  8.73264194e-01,  7.20662057e-01,  5.46057701e-01,
        3.35043997e-01,  7.72289485e-02, -1.95161819e-01, -4.67396259e-01,
       -7.27581799e-01, -9.47028518e-01, -1.10465205e+00, -1.20025849e+00,
       -1.21789014e+00, -1.15382624e+00, -1.02124512e+00, -8.31534922e-01,
       -6.00374758e-01, -3.52129310e-01, -9.54796821e-02,  1.76716357e-01,
        4.47278470e-01,  6.77465796e-01,  8.68327796e-01,  1.02752829e+00,
        1.12877262e+00,  1.16390038e+00,  1.15785170e+00,  1.10008824e+00,
        9.87799227e-01,  8.51269543e-01,  7.02306569e-01,  5.19140124e-01,
        3.13696593e-01,  1.13218725e-01, -6.19748086e-02, -2.42193088e-01,
       -4.49979514e-01, -6.42615855e-01, -7.57645845e-01, -8.30896676e-01,
       -8.71494293e-01, -8.68848085e-01, -8.17958295e-01, -7.20479786e-01,
       -6.05775058e-01, -4.90818202e-01, -3.49814653e-01, -2.10367307e-01,
       -7.60469735e-02,  5.90579957e-02,  2.04065785e-01,  3.51087004e-01,
        4.74104792e-01,  5.41678309e-01,  5.77240884e-01,  5.81530273e-01,
        5.32142818e-01,  4.60485548e-01,  3.78359944e-01,  2.97356993e-01,
        2.31455013e-01,  1.79687202e-01,  1.29356101e-01,  6.86916336e-02,
        3.57055082e-03, -7.46511593e-02, -1.46855950e-01, -2.07633764e-01,
       -2.65349716e-01, -3.57461244e-01, -3.72807801e-01, -4.12545919e-01,
       -4.42624718e-01, -4.67767298e-01, -4.71117496e-01, -4.76361960e-01,
       -4.57290649e-01, -4.12008643e-01, -3.52056891e-01, -2.68756658e-01,
       -1.38320729e-01,  2.28515547e-03,  1.53052613e-01,  2.94536322e-01,
        4.21719551e-01,  4.95520324e-01,  5.33840537e-01,  5.30682325e-01,
        4.91883695e-01,  4.34849054e-01,  3.63785833e-01,  2.75893688e-01,
        1.75473914e-01,  6.62139058e-02, -3.10708918e-02, -1.40731812e-01,
       -2.49430940e-01, -3.62833679e-01, -4.68847007e-01, -5.69800436e-01,
       -6.95437014e-01, -7.80018747e-01, -8.42027724e-01, -8.79551828e-01,
       -8.78588200e-01, -8.41824293e-01, -7.61113405e-01, -6.32699847e-01,
       -4.69106376e-01, -2.80145347e-01, -8.37900788e-02,  1.09752998e-01,
        2.82698154e-01,  4.48713243e-01,  5.83988845e-01,  6.79491401e-01,
        7.81081021e-01,  8.58245611e-01,  9.02000546e-01,  9.00450289e-01,
        8.54837179e-01,  7.66643941e-01,  6.41260684e-01,  4.64135647e-01,
        2.68213958e-01,  6.60439208e-02, -1.48352727e-01, -3.32949668e-01,
       -5.02753437e-01, -6.30851328e-01, -7.22329855e-01, -8.26332748e-01,
       -9.21969056e-01, -9.70997334e-01, -1.02132344e+00, -1.04646444e+00,
       -1.04365754e+00, -1.00255537e+00, -9.08393621e-01, -7.67155945e-01,
       -5.70251942e-01, -3.16171885e-01, -5.08606508e-02,  2.07581207e-01,
        4.47287619e-01,  6.39969945e-01,  7.75292933e-01,  8.64163399e-01,
        9.08495486e-01,  9.06138778e-01,  8.73185575e-01,  8.29844475e-01,
        7.74959385e-01,  7.00181901e-01,  6.12186849e-01,  5.00542104e-01,
        3.51471215e-01,  1.82107896e-01,  1.24993268e-02, -1.68922648e-01,
       -3.37497681e-01, -4.80124980e-01, -6.00163937e-01, -6.90507472e-01,
       -7.52968311e-01, -7.81891048e-01, -7.85357296e-01, -7.63447821e-01,
       -7.09662199e-01, -6.24309778e-01, -5.25793612e-01, -4.20651942e-01,
       -2.93795556e-01, -1.58203796e-01, -2.54646167e-02,  1.20985962e-01,
        2.66110420e-01,  3.83559406e-01,  4.85191226e-01,  5.52343488e-01,
        5.86530566e-01,  5.80043256e-01,  5.38517773e-01,  4.60318446e-01,
        3.52688670e-01,  2.35098436e-01,  1.29022732e-01,  3.60062867e-02,
       -3.90242115e-02, -1.00243099e-01, -1.53216809e-01, -2.15854183e-01,
       -2.76589632e-01, -3.39056671e-01, -4.20653433e-01, -4.60575491e-01,
       -4.82642293e-01, -4.79454964e-01, -4.37229782e-01, -3.64975542e-01,
       -2.81643212e-01, -1.82426497e-01, -7.19822049e-02,  3.69194709e-02,
        1.46373406e-01,  2.52830565e-01,  3.46243709e-01,  4.24359679e-01,
        4.83362228e-01,  5.18551350e-01,  5.25191963e-01,  5.05261004e-01,
        4.65113938e-01,  3.87747765e-01,  2.84057558e-01,  1.62732795e-01,
        3.30562927e-02, -1.00562632e-01, -2.23115593e-01, -3.25732261e-01,
       -4.10646021e-01, -4.79867429e-01, -5.33685207e-01, -5.73973894e-01,
       -5.95835686e-01, -5.89473069e-01, -5.58612704e-01, -5.01861155e-01,
       -4.07437235e-01, -2.82926321e-01, -1.43569961e-01, -2.45555886e-04,
        1.46605760e-01,  2.84016550e-01,  4.02980953e-01,  5.06471395e-01,
        5.80066562e-01,  6.29524589e-01,  6.51383638e-01,  6.43274009e-01,
        6.08629346e-01,  5.50780416e-01,  4.67557520e-01,  3.67258489e-01,
        2.43098140e-01,  9.87629518e-02, -6.03372864e-02, -2.33468577e-01,
       -3.90901059e-01, -5.20207405e-01, -6.23266816e-01, -6.81652188e-01,
       -7.00849414e-01, -6.87250495e-01, -6.47266209e-01, -5.85918486e-01,
       -5.06169140e-01, -4.17739123e-01, -3.23758543e-01, -2.16253668e-01,
       -1.04981199e-01,  1.83534157e-02,  1.68594599e-01,  2.92059511e-01,
        3.67524385e-01,  4.95508373e-01,  5.98136127e-01,  6.49490297e-01,
        6.78188384e-01,  6.65597022e-01,  6.23024702e-01,  5.43376565e-01,
        4.33284402e-01,  2.98194200e-01,  1.57272711e-01,  1.30788246e-02,
       -1.21843390e-01, -2.22666085e-01, -2.84513444e-01, -3.59019518e-01,
       -4.46824044e-01, -4.78453338e-01, -5.30836880e-01, -5.55043578e-01,
       -5.70579410e-01, -5.58597624e-01, -5.19894838e-01, -4.64417130e-01,
       -3.93958420e-01, -3.06287199e-01, -2.20633462e-01, -1.23678312e-01,
       -3.57281417e-02,  4.68527824e-02,  1.35542884e-01,  1.91786632e-01,
        2.56906271e-01,  3.16889644e-01,  3.66028547e-01,  4.04170334e-01,
        4.23061162e-01,  4.16997820e-01,  3.95144522e-01,  3.49894255e-01,
        2.88541138e-01,  2.06056312e-01,  1.22068271e-01,  3.61676998e-02,
       -5.50978445e-02, -1.17589749e-01, -1.69353724e-01, -2.49813586e-01,
       -2.97429353e-01, -3.09523463e-01, -3.01411688e-01, -2.93891162e-01,
       -2.81411499e-01, -2.57003516e-01, -2.40117550e-01, -2.30215862e-01,
       -2.20901221e-01, -2.22683892e-01, -2.16655746e-01, -2.05594733e-01      
};



// The name of this function is important for Arduino compatibility.
void setup() {
  Serial.begin(9600);
  Serial.println("Entry point...");
  tflite::InitializeTarget();
  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
  }else{
    Serial.println("Model schema version is fine.");
  }


  // This pulls in all the operation implementations we need.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::AllOpsResolver resolver;

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  delay(3000);
  if (allocate_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
  }else{
        Serial.println("Tensor allocation is fine.");
  }

  // Obtain pointers to the model's input and output tensors.

  input = interpreter->input(0);
  output = interpreter->output(0);


   input->data.f = example_signal;
  // Keep track of how many inferences we have performed.

}

// The name of this function is important for Arduino compatibility.
void loop() {
  Serial.println("1\n");
  // Serial.println(count1);
  // Serial.println(count2);
  // Serial.println(count3);
  // Serial.println(count4); 
  // Serial.println(count5);
  // Serial.println(count6);
  // Serial.println(count7);
  delay(1000);
  
//define input
  // for (size_t i = 0; i < 1000; i++)
  // {
  //   /* code */model_input_buffer[i] = example_signal[i];
  // }
  

  
  Serial.println("2\n");
  delay(1000);
  TfLiteStatus invoke_status = interpreter->Invoke();
  // Run inference, and report any error
  if (invoke_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed in Invoking Progress\n");
  }else{
      Serial.println("Invocation is fine.");
  }
  Serial.println("3\n");
  delay(1000);
  // define output
  Serial.println("type 1\n");
  float value0 = output->data.f[0];
  Serial.println(value0);

  Serial.println("type 2\n");
  float value1 = output->data.f[1];
  Serial.println(value1);

  Serial.println("type 3\n");
  float value2 = output->data.f[2];
  Serial.println(value2);

  
  // Increment the inference_counter, and reset it if we have reached
  // the total number per cycle
  delay(50);

}
