USE [dnworld]
GO
/****** Object:  Table [dbo].[FieldsForCharacter]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[FieldsForCharacter](
	[CharacterID] [bigint] NOT NULL,
	[FieldIndex] [smallint] NOT NULL,
	[ItemID] [int] NOT NULL,
	[PlantDate] [datetime2](0) NOT NULL,
	[ElapsedTime] [int] NOT NULL,
	[LastUpdateDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_FieldsForCharacter] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[FieldIndex] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
