USE [dnworld]
GO
/****** Object:  Table [dbo].[PVPTournamentIDs]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PVPTournamentIDs](
	[PVPTournamentID] [bigint] IDENTITY(1,1) NOT NULL,
	[PVPTournamentResultDate] [datetime2](0) NOT NULL,
	[PlayerCount] [tinyint] NOT NULL,
	[GambleID] [bigint] NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NC_PVPTournamentIDs_PVPTournamentResultDate]    Script Date: 11/14/2026 8:54:16 AM ******/
CREATE CLUSTERED INDEX [IX_NC_PVPTournamentIDs_PVPTournamentResultDate] ON [dbo].[PVPTournamentIDs]
(
	[PVPTournamentResultDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [PK_PVPTournamentIDs]    Script Date: 11/14/2026 8:54:37 AM ******/
ALTER TABLE [dbo].[PVPTournamentIDs] ADD  CONSTRAINT [PK_PVPTournamentIDs] PRIMARY KEY NONCLUSTERED 
(
	[PVPTournamentID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_UNF_PVPTournamentIDs_GambleID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UNF_PVPTournamentIDs_GambleID] ON [dbo].[PVPTournamentIDs]
(
	[GambleID] ASC
)
WHERE ([GambleID] IS NOT NULL AND [GambleID]>(0))
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
