<template>
  <div class="juce-combobox">
    <!-- You can optionally show a label here later -->
    <select :value="modelValue" @change="onChange" class="combobox-select">
      <option v-for="(option, index) in options" :key="index" :value="index">
        {{ option }}
      </option>
    </select>
  </div>
</template>

<script setup lang="ts">
defineProps<{
  modelValue: number; // selected index
  options: string[]; // list of options
}>();

const emit = defineEmits<{
  (e: "update:modelValue", value: number): void;
}>();

function onChange(e: Event) {
  const target = e.target as HTMLSelectElement;
  const newValue = parseInt(target.value, 10);
  console.log("ComboBox changed:", newValue);
  emit("update:modelValue", newValue);
}
</script>

<style scoped>
.juce-combobox {
  display: flex;
  flex-direction: column;
  align-items: center;
  width: 100%; /* panel will control final width */
}

/* Styled <select> */
.combobox-select {
  width: 100%; /* Full width inside container */
  padding: 8px;
  background: linear-gradient(to bottom, #444, #222);
  box-shadow: inset 0 2px 5px rgba(0, 0, 0, 0.5);
  color: white;
  border: 1px solid #333; /* thin dark border */
  border-radius: 0px; /* NO rounded corners */
  font-size: 0.8rem;
  appearance: none; /* Hide native select arrow in some browsers */
  -webkit-appearance: none;
  -moz-appearance: none;
  cursor: pointer;
}

/* Style the options themselves */
.combobox-select option {
  border-radius: none;
  background: linear-gradient(to bottom, #444, #222);
  box-shadow: inset 0 2px 5px rgba(0, 0, 0, 0.5);
  color: white;
}
</style>
