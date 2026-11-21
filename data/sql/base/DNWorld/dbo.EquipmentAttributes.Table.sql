USE [dnworld]
GO
/****** Object:  Table [dbo].[EquipmentAttributes]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[EquipmentAttributes](
	[CharacterID] [bigint] NOT NULL,
	[EquipmentCode] [tinyint] NOT NULL,
	[EquipmentAttributeCode] [tinyint] NOT NULL,
	[AttributeValue] [int] NOT NULL,
 CONSTRAINT [PK_EquipmentAttributes] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[EquipmentCode] ASC,
	[EquipmentAttributeCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
