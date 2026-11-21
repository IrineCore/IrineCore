USE [dnworld]
GO
/****** Object:  Table [dbo].[AlteiaSendTickets]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AlteiaSendTickets](
	[CharacterID] [bigint] NOT NULL,
	[ReceiveCharacterID] [bigint] NOT NULL,
	[SendDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_AlteiaSendTickets] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[ReceiveCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_AlteiaSendTickets_ReceiveCharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_AlteiaSendTickets_ReceiveCharacterID] ON [dbo].[AlteiaSendTickets]
(
	[ReceiveCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
