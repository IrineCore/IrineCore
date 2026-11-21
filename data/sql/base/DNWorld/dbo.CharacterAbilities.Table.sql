USE [dnworld]
GO
/****** Object:  Table [dbo].[CharacterAbilities]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CharacterAbilities](
	[CharacterID] [bigint] NOT NULL,
	[CharacterAbilityCode] [tinyint] NOT NULL,
	[AbilityPoint] [int] NOT NULL,
 CONSTRAINT [PK_CharacterAbilities] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[CharacterAbilityCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
