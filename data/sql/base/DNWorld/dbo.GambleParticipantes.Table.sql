USE [dnworld]
GO
/****** Object:  Table [dbo].[GambleParticipantes]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GambleParticipantes](
	[GambleID] [bigint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
 CONSTRAINT [PK_GambleParticipantes] PRIMARY KEY CLUSTERED 
(
	[GambleID] ASC,
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[GambleParticipantes]  WITH CHECK ADD  CONSTRAINT [FK_GambleIDs_GambleParticipantes_GambleID] FOREIGN KEY([GambleID])
REFERENCES [dbo].[GambleIDs] ([GambleID])
GO
ALTER TABLE [dbo].[GambleParticipantes] CHECK CONSTRAINT [FK_GambleIDs_GambleParticipantes_GambleID]
GO
