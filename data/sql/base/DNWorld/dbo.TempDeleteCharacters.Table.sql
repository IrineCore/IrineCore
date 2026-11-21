USE [dnworld]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TempDeleteCharacters](
	[CharacterID] [bigint] NULL,
	[DeleteYN] [tinyint] NULL
)
GO
CREATE CLUSTERED INDEX [IX_NC_TempDeleteCharacters_DeleteYN_CharacterID] ON [dbo].[TempDeleteCharacters]
(
	[DeleteYN] ASC,
	[CharacterID] ASC
) ON [IndexFG1]
GO
