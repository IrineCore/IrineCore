USE [dnmembership]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TargetCharacterDetails](
	[Seq] [int] IDENTITY(1, 1) NOT NULL PRIMARY KEY,
	[CharacterID] [bigint] NOT NULL,
	[CompleteFlag] [bit] NOT NULL DEFAULT (0)
)
GO
CREATE UNIQUE INDEX [IX_UN_TargetCharacterDetails_CharacterID] ON [dbo].[TargetCharacterDetails]
(
	[CharacterID] ASC
) ON [IndexFG1]
GO
