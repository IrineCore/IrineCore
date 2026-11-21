USE [dnstaging]
GO
/****** Object:  Table [dbo].[EventResults]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[EventResults](
	[EventID] [int] NOT NULL,
	[BasisDate] [date] NOT NULL,
	[EventSubID] [int] NOT NULL,
	[AccountID] [int] SPARSE  NULL,
	[AccountName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS SPARSE  NULL,
	[CharacterID] [int] SPARSE  NULL,
	[CharacterName] [nvarchar](30) COLLATE Chinese_PRC_CI_AS SPARSE  NULL,
	[WorldID] [int] SPARSE  NULL,
	[CharacterClassCode] [tinyint] NULL,
	[JobCode] [tinyint] NULL,
	[CharacterLevel] [tinyint] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_EventResults_EventID_BasisDate_EventSubID]    Script Date: 11/14/2026 9:00:09 AM ******/
CREATE CLUSTERED INDEX [IX_NC_EventResults_EventID_BasisDate_EventSubID] ON [dbo].[EventResults]
(
	[EventID] ASC,
	[BasisDate] ASC,
	[EventSubID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[EventResults]  WITH CHECK ADD  CONSTRAINT [FK_Events_EventResults_EventID] FOREIGN KEY([EventID])
REFERENCES [dbo].[Events] ([EventID])
GO
ALTER TABLE [dbo].[EventResults] CHECK CONSTRAINT [FK_Events_EventResults_EventID]
GO
