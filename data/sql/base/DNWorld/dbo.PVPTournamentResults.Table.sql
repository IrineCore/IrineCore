USE [dnworld]
GO
/****** Object:  Table [dbo].[PVPTournamentResults]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PVPTournamentResults](
	[PVPTournamentID] [bigint] NOT NULL,
	[PVPTournamentSeqence] [tinyint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[FinalRound] [tinyint] NOT NULL,
	[ResultFlag] [bit] NOT NULL,
 CONSTRAINT [PK_PVPTournamentResults] PRIMARY KEY CLUSTERED 
(
	[PVPTournamentID] ASC,
	[PVPTournamentSeqence] ASC,
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[PVPTournamentResults]  WITH CHECK ADD  CONSTRAINT [FK_PVPTournamentIDs_PVPTournamentResults_PVPTournamentID] FOREIGN KEY([PVPTournamentID])
REFERENCES [dbo].[PVPTournamentIDs] ([PVPTournamentID])
GO
ALTER TABLE [dbo].[PVPTournamentResults] CHECK CONSTRAINT [FK_PVPTournamentIDs_PVPTournamentResults_PVPTournamentID]
GO
