USE [dnstaging]
GO
/****** Object:  Table [dbo].[PartyLogs_20131005]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PartyLogs_20131005](
	[PartyStartDate] [datetime2](0) NOT NULL,
	[PartyEndDate] [datetime2](0) NULL,
	[PartyStartID] [bigint] NOT NULL,
	[WorldID] [int] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_PartyLogs_20131005_PartyStartDate]    Script Date: 11/14/2026 9:00:08 AM ******/
CREATE CLUSTERED INDEX [IX_NC_PartyLogs_20131005_PartyStartDate] ON [dbo].[PartyLogs_20131005]
(
	[PartyStartDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[PartyLogs_20131005]  WITH CHECK ADD  CONSTRAINT [CK_PartyLogs_20131005_PartyStartDate] CHECK  (([PartyStartDate]>='2013-10-05' AND [PartyStartDate]<'2013-10-06'))
GO
ALTER TABLE [dbo].[PartyLogs_20131005] CHECK CONSTRAINT [CK_PartyLogs_20131005_PartyStartDate]
GO
