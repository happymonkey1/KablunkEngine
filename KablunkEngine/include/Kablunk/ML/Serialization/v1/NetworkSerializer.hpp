#pragma once
#ifndef KABLUNK_ML_SERIALIZATION_V1_NETWORK_SERIALIZER_HPP
#define KABLUNK_ML_SERIALIZATION_V1_NETWORK_SERIALIZER_HPP

#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/ML/Network/INetwork.hpp"
#include "Kablunk/ML/Network/ILayer.hpp"

#include <yaml-cpp/yaml.h>

#include <filesystem>

namespace kb::ml::serialize::v1
{

	template <typename T = f32>
	class NetworkSerializer
	{
	public:
		using value_t = T;
		using network_t = network::INetwork<value_t>;
		using layer_t = network::ILayer<value_t, network::INetwork<value_t>::network_tensor_t>;
	public:

		/*	
		 *  \brief serialize network to a file
		 *	\param filepath path to file where the network will be serialized
		 *	\param network pointer to the base class of the network that will be serialized
		 *	\details example serialization for a network: 4x3x3x1
		 *	============================================
		 *	architecture:
		 *		layer_count: 3
		 *		layer1:
		 *			input_size: 4
		 *			output_size: 3
		 *			weights:
		 *				w0, w1, ..., wn
		 *		layer2:
		 *			input_size: 3
		 *			output_size: 3
		 *			weights:
		 *				w0, w1, ..., wn
		 *		layer3:
		 *			input_size: 3
		 *			output_size: 3
		 *			weights:
		 *				w0, w1, ..., wn
		 *		layer4:
		 *			input_size: 3
		 *			output_size: 1
		 *			weights:
		 *				w0, w1, ..,. wn
		 *	============================================
		 */
		void serialize(const std::filesystem::path& filepath, const network_t* network) const;
		
		/*	
		 *  \brief serialize network to a file
		 *	\param filepath path to file where the network will be deserialized from
		 *	\param network pointer to the base class of the network that will be deserialized into
		 */ 
		void deserialize(const std::filesystem::path& filepath, network_t* network) const;

		void serialize_binary(const std::filesystem::path& filepath, const network_t* network) const
		{
			KB_CORE_ASSERT(false, "not implemented");
		}

		void deserialize_binary(const std::filesystem::path& filepath, network_t* network) const
		{
			KB_CORE_ASSERT(false, "not implemented");
		}

	};

	template <typename T /*= f32*/>
	void NetworkSerializer<T>::serialize(const std::filesystem::path& filepath, const network_t* network) const
	{
		// write file using yaml
		YAML::Emitter out;
		out << YAML::BeginMap;

		// write header for the file
		out << YAML::Key << "architecture";
		out << YAML::BeginMap;
		
		out << YAML::Key << "layer_count" << YAML::Value << network->get_layer_count();
		out << YAML::BeginSeq;
		for (size_t i = 0; i < network->get_layer_count(); ++i)
		{
			const layer_t* layer = network->get_layer(i);
			out << YAML::Key << fmt::format("layer{}", i);
			out << YAML::BeginMap;
			out << YAML::Key << "input_size" << YAML::Value << layer->get_input_node_count();
			out << YAML::Key << "output_size" << YAML::Value << layer->get_output_node_count();

			// write weights as one long list
			// #TODO should this be in matrix form?
			out << YAML::Key << "weights";
			out << YAML::Flow << YAML::BeginSeq;
			for (value_t w : layer->get_weights())
				out << w;
			out << YAML::EndSeq;

			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		// #TODO filesystem stuff

		// write yaml representation of network to file
		std::ofstream fout{ filepath };
		fout << out.c_str();
	}

	template <typename T /*= f32*/>
	void NetworkSerializer<T>::deserialize(const std::filesystem::path& filepath, network_t* network) const
	{

	}

}

#endif
