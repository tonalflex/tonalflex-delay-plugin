<template>
  <div class="plugin-container">
    <h4 class="headline">TONALFLEX DELAY</h4>

    <div class="parameter-grid">
      <div class="parameter">
        <ToggleControl label="Tempo Sync" v-model="sync" />
      </div>
      <div class="parameter">
        <div>
          <SevenSegmentDisplay
            :value="Math.round(500 + hiCutFreq * (16000 - 500))"
          />
          <span class="display-text">hz</span>
        </div>
        <SliderControl
          label="Hi Cut"
          v-model="hiCutFreq"
          :knob-image="knobWhite"
          :frames="79"
        />
      </div>
    </div>

    <div class="parameter-grid">
      <div class="parameter">
        <ComboBoxControl
          label="Division"
          v-model="division"
          :options="['1/1', '1/2', '1/4', '1/8', '1/8 Dotted', '1/16']"
        />
      </div>
      <div class="parameter">
        <ComboBoxControl
          label="Mode"
          v-model="mode"
          :options="['Mono', 'Stereo', 'Ping Pong']"
        />
      </div>
    </div>

    <div class="parameter-grid">
      <div class="parameter">
        <div>
          <SevenSegmentDisplay :value="Math.round(delayTime * 2000)" />
          <span class="display-text">ms</span>
        </div>
        <SliderControl
          label="Delay Time"
          v-model="delayTime"
          :knob-image="knobGreen"
          :frames="79"
        />
      </div>
      <div class="parameter">
        <div>
          <SevenSegmentDisplay :value="Math.round(feedback * 100)" />
          <span class="display-text">%</span>
        </div>
        <SliderControl
          label="Feedback"
          v-model="feedback"
          :knob-image="knobGreen"
          :frames="79"
        />
      </div>
    </div>

    <div class="parameter-grid">
      <div class="parameter">
        <div><SevenSegmentDisplay :value="Math.round(dryLevel * 100)" />%</div>
        <SliderControl
          label="Dry Level"
          v-model="dryLevel"
          :knob-image="knobWhite"
          :frames="79"
        />
      </div>
      <div class="parameter">
        <div><SevenSegmentDisplay :value="Math.round(wetLevel * 100)" />%</div>
        <SliderControl
          label="Wet Level"
          v-model="wetLevel"
          :knob-image="knobWhite"
          :frames="79"
        />
      </div>
    </div>

    <div class="parameter-grid">
      <div class="parameter">
        <div><SevenSegmentDisplay :value="Math.round(modRate * 100)" />%</div>
        <SliderControl
          label="Mod Rate"
          v-model="modRate"
          :knob-image="knobBlue"
          :frames="79"
        />
      </div>
      <div class="parameter">
        <div><SevenSegmentDisplay :value="Math.round(modDepth * 100)" />%</div>
        <SliderControl
          label="Mod Depth"
          v-model="modDepth"
          :knob-image="knobBlue"
          :frames="79"
        />
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import SliderControl from "@components/controls/SliderControl.vue";
import { useParameter } from "@composables/useParameter";
import ToggleControl from "./controls/ToggleControl.vue";
import ComboBoxControl from "./controls/ComboBoxControl.vue";
import knobGreen from "@/assets/knob_njd_green.png?inline";
import knobWhite from "@/assets/knob_njd_white.png?inline";
import knobBlue from "@/assets/knob_njd_blue.png?inline";
import { SevenSegmentDisplay } from "vue3-seven-segment-display";

// Plugin Parameters
const hiCutFreq = useParameter("hiCutFreq", "slider");
const delayTime = useParameter("delayTime", "slider");
const feedback = useParameter("feedback", "slider");
const wetLevel = useParameter("wetLevel", "slider");
const dryLevel = useParameter("dryLevel", "slider");
const modDepth = useParameter("modDepth", "slider");
const modRate = useParameter("modRate", "slider");
const sync = useParameter("sync", "toggle");
const division = useParameter("division", "comboBox");
const mode = useParameter("mode", "comboBox");
</script>

<!-- Use style.css to change global styles such as font, colors, etc. -->
<style scoped>
.headline {
  font-size: 2rem;
  font-family: "Digital", monospace;
  text-align: center;
  margin-bottom: 1rem;
  background: linear-gradient(90deg, #29d860, #00cc66, #44ec71);
  background-clip: text;
  -webkit-background-clip: text;
  color: transparent;
  -webkit-text-fill-color: transparent;
  font-weight: normal;
  text-shadow: 0 0 3px rgba(0, 255, 0, 0.5);
  user-select: none;
}
.display-text {
  font-family: "Digital", monospace;
  font-size: 1.2rem;
  font-weight: bold;
}
.plugin-container {
  display: flex;
  flex-direction: column;
  gap: 0rem;
  align-items: center;
  justify-content: center;
  height: 100vh;
}
.parameter-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 3rem;
  margin-bottom: 1.5rem;
  justify-content: center;
  width: fit-content;
  max-width: 300px;
  width: 100%;
}
.parameter {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  user-select: none;
}
</style>
