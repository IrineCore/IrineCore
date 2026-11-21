USE [dnworld]
GO
/****** Object:  Table [dbo].[PartyStartIDs]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PartyStartIDs](
	[PartyStartID] [bigint] IDENTITY(1,1) NOT NULL,
	[PartyID] [bigint] NOT NULL,
	[PartyStartDate] [datetime2](0) NOT NULL,
	[PartyEndDate] [datetime2](0) NULL,
 CONSTRAINT [PK_PartyStartIDs] PRIMARY KEY CLUSTERED 
(
	[PartyStartID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_PartyStartIDs_PartyID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PartyStartIDs_PartyID] ON [dbo].[PartyStartIDs]
(
	[PartyID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_PartyStartIDs_PartyStartDate]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PartyStartIDs_PartyStartDate] ON [dbo].[PartyStartIDs]
(
	[PartyStartDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
