USE [dnworld]
GO
/****** Object:  Table [dbo].[NPCFavor]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[NPCFavor](
	[CharacterID] [bigint] NOT NULL,
	[NPCID] [int] NOT NULL,
	[FavorPoint] [smallint] NOT NULL,
	[MalicePoint] [smallint] NOT NULL,
 CONSTRAINT [PK_NPCFavor] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[NPCID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
