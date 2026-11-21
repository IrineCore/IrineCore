USE [dnworld]
GO
/****** Object:  Table [dbo].[PVPLadderKillDeathByJob]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PVPLadderKillDeathByJob](
	[CharacterID] [bigint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[KOWin] [int] NOT NULL,
	[KOLose] [int] NOT NULL,
 CONSTRAINT [PK_PVPLadderKillDeathByJob] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[JobCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_PVPLadderKillDeathByJob_CharacterID_KOLose]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PVPLadderKillDeathByJob_CharacterID_KOLose] ON [dbo].[PVPLadderKillDeathByJob]
(
	[CharacterID] ASC,
	[KOLose] DESC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_PVPLadderKillDeathByJob_CharacterID_KOWin]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PVPLadderKillDeathByJob_CharacterID_KOWin] ON [dbo].[PVPLadderKillDeathByJob]
(
	[CharacterID] ASC,
	[KOWin] DESC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
