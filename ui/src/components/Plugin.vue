<template>
  <div class="plugin-container">
    <img :src="logo" alt="Logo" class="logo" style="max-width: 250px" />

    <div class="parameter-grid">
      <div class="parameter">
        <ToggleControl label="Sync" v-model="sync" />
      </div>
      <div class="parameter">
        <ComboBoxControl
          label="Division"
          v-model="division"
          :options="['1/1', '1/2', '1/4', '1/8', '1/8 Dotted', '1/16']"
        />
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
          <SevenSegmentDisplay :value="Math.round(delayTime * 2000)" />ms
        </div>
        <SliderControl
          label="Delay Time"
          v-model="delayTime"
          :knob-image="knobGreen"
          :frames="79"
        />
      </div>
      <div class="parameter">
        <div><SevenSegmentDisplay :value="Math.round(feedback * 100)" />%</div>
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
        <div><SevenSegmentDisplay :value="Math.round(wetLevel * 100)" />%</div>
        <SliderControl
          label="Wet Level"
          v-model="wetLevel"
          :knob-image="knobWhite"
          :frames="79"
        />
      </div>
      <div class="parameter">
        <div><SevenSegmentDisplay :value="Math.round(dryLevel * 100)" />%</div>
        <SliderControl
          label="Dry Level"
          v-model="dryLevel"
          :knob-image="knobWhite"
          :frames="79"
        />
      </div>
    </div>

    <div class="parameter-grid">
      <div class="parameter">
        <div><SevenSegmentDisplay :value="Math.round(modDepth * 100)" />%</div>
        <SliderControl
          label="Modulation Depth"
          v-model="modDepth"
          :knob-image="knobBlue"
          :frames="79"
        />
      </div>
      <div class="parameter">
        <div><SevenSegmentDisplay :value="Math.round(modRate * 100)" />%</div>
        <SliderControl
          label="Modulation Rate"
          v-model="modRate"
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
import logo from "@/assets/logo.png?inline";
import ToggleControl from "./controls/ToggleControl.vue";
import ComboBoxControl from "./controls/ComboBoxControl.vue";
import knobGreen from "@/assets/knob_njd_green.png?inline";
import knobWhite from "@/assets/knob_njd_white.png?inline";
import knobBlue from "@/assets/knob_njd_blue.png?inline";
import { SevenSegmentDisplay } from "vue3-seven-segment-display";

// Plugin Parameters
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
.logo {
  width: 60vw;
  max-width: 400px;
  height: auto;
}
.plugin-container {
  display: flex;
  flex-direction: column;
  gap: 0.5rem;
  align-items: center;
  justify-content: center;
  height: 100vh;
}

.parameter-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 2rem;
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
}
</style>
