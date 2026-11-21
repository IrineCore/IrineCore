USE [dnserverlog]
GO
/****** Object:  Table [dbo].[DatabaseVersion]    Script Date: 11/14/2026 8:57:59 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DatabaseVersion](
	[version] [int] NOT NULL,
	[registerDate] [datetime] NOT NULL,
 CONSTRAINT [PK_DatabaseVersion] PRIMARY KEY CLUSTERED 
(
	[version] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
INSERT [dbo].[DatabaseVersion] ([version], [registerDate]) VALUES (18, CAST(N'2013-09-13T08:54:22.667' AS DateTime))
GO
ALTER TABLE [dbo].[DatabaseVersion] ADD  CONSTRAINT [DF_DatabaseVersion]  DEFAULT (getdate()) FOR [registerDate]
GO
